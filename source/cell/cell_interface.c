/*******************************************************************************
FILE : cell_interface.c

LAST MODIFIED : 04 April 2001

DESCRIPTION :
The Cell Interface.
==============================================================================*/
#include <stdio.h>

/* The XML headers */
#include "local_utils/xerces_wrapper.h"

#include "cell/cell_calculate.h"
#include "cell/cell_cmgui_interface.h"
#include "cell/cell_component.h"
#include "cell/cell_export_dialog.h"
#include "cell/cell_graphic.h"
#include "cell/cell_input.h"
#include "cell/cell_interface.h"
#include "cell/cell_output.h"
#include "cell/cell_unemap_interface.h"
#include "cell/cell_variable.h"
#include "cell/cell_variable_editing_dialog.h"
#include "cell/cell_window.h"
#include "cell/distributed_editing_interface.h"

/*
Module objects
--------------
*/
struct Cell_interface
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
The Cell Interface main structure.
==============================================================================*/
{
  /* used to keep track of the status of the XML parser.
   * NOTE: the Xerces parser can only be initialised once and terminated
   *       once per application!! So tricky stuff is done :-)
   */
  int XMLParser_initialised;
  /* The main window for the interface */
  struct Cell_window *window;
  /* The structure used for all file (?) input into the Cell interface */
  struct Cell_input *input;
  /* The list of all the component objects for the current model contained
   * in the Cell interface
   */
  struct LIST(Cell_component) *component_list;
  /* The list of all the variable objects for the current model contained in the
   * Cell interface
   */
  struct LIST(Cell_variable) *variable_list;
  /* The list of all the graphic objects for the current model contained in the
   * Cell interface
   */
  struct LIST(Cell_graphic) *graphic_list;
  /* The user interface structure - is it needed here ?? */
  struct User_interface *user_interface;
  /* The model calculation object */
  struct Cell_calculate *calculate;
  /* The UnEMAP interface object */
  struct Cell_unemap_interface *unemap_interface;
  /* The variable editing dialog */
  struct Cell_variable_editing_dialog *variable_editing_dialog;
  /* The time keeper */
  struct Time_keeper *time_keeper;
  /* The interface to CMGUI objects */
  struct Cell_cmgui_interface *cmgui_interface;
  /* The interface for distributed parameter editing */
  struct Distributed_editing_interface *distributed_editing_interface;
#if defined (CELL_DISTRIBUTED)
  /* The dialog used to export to CMISS ipcell and ipmatc files */
  struct Cell_export_dialog *export_dialog;
#endif /* defined (CELL_DISTRIBUTED) */
}; /* struct Cell_interface */

/*
Global functions
----------------
*/
struct Cell_interface *CREATE(Cell_interface)(
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
  XtCallbackProc exit_callback
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
Creates the Cell interface object.
==============================================================================*/
{
  struct Cell_interface *cell_interface;

  ENTER(CREATE(Cell_interface));
  if (any_object_selection && background_colour &&
    default_graphical_material && default_light && default_light_model &&
    default_scene && default_spectrum && time_keeper &&
    graphics_object_list && glyph_list && interactive_tool_manager &&
    light_manager && light_model_manager && graphical_material_manager &&
    scene_manager && spectrum_manager && texture_manager && user_interface)
  {
    if (ALLOCATE(cell_interface,struct Cell_interface,1))
    {
      /* initialise data objects */
      cell_interface->time_keeper = ACCESS(Time_keeper)(time_keeper);
      cell_interface->user_interface = user_interface;
      cell_interface->XMLParser_initialised = 0;
      cell_interface->window = (struct Cell_window *)NULL;
      cell_interface->input = (struct Cell_input *)NULL;
      cell_interface->calculate = (struct Cell_calculate *)NULL;
      cell_interface->unemap_interface = (struct Cell_unemap_interface *)NULL;
      cell_interface->component_list = (struct LIST(Cell_component) *)NULL;
      cell_interface->variable_list = (struct LIST(Cell_variable) *)NULL;
      cell_interface->graphic_list = (struct LIST(Cell_graphic) *)NULL;
      cell_interface->variable_editing_dialog =
        (struct Cell_variable_editing_dialog *)NULL;
      cell_interface->cmgui_interface = (struct Cell_cmgui_interface *)NULL;
      cell_interface->distributed_editing_interface =
        (struct Distributed_editing_interface *)NULL;
#if defined (CELL_DISTRIBUTED)
      cell_interface->export_dialog = (struct Cell_export_dialog *)NULL;
#endif /* defined (CELL_DISTRIBUTED) */
      /* create the main window */
      if (cell_interface->window = CREATE(Cell_window)(cell_interface,
        user_interface,exit_callback))
      {
        /* create the cell input object */
        if (cell_interface->input = CREATE(Cell_input)())
        { 
          /* create the cell component list */
          if (cell_interface->component_list = CREATE(LIST(Cell_component))())
          {
            /* create the cell variable list */
            if (cell_interface->variable_list = CREATE(LIST(Cell_variable))())
            {
              /* create the cell graphic list */
              if (cell_interface->graphic_list = CREATE(LIST(Cell_graphic))())
              {
                if (cell_interface->unemap_interface =
                  CREATE(Cell_unemap_interface)(
                    Cell_window_get_shell(cell_interface->window),
                    Cell_window_get_unemap_activation_button(
                      cell_interface->window),
                    cell_interface->time_keeper,
                    cell_interface->user_interface))
                {
                  if (cell_interface->cmgui_interface =
                    CREATE(Cell_cmgui_interface)(any_object_selection,
                      background_colour,
                      default_graphical_material,
                      default_light,default_light_model,default_scene,
                      default_spectrum,time_keeper,graphics_object_list,
                      glyph_list,interactive_tool_manager,light_manager,
                      light_model_manager,graphical_material_manager,
                      scene_manager,spectrum_manager,texture_manager,
                      user_interface,cell_interface->window,cell_interface
#if defined (CELL_DISTRIBUTED)
                      ,element_point_ranges_selection,computed_field_package,
                      element_manager,element_group_manager,fe_field_manager
#endif /* defined (CELL_DISTRIBUTED) */
                      ))
                  {
                    /* Everything created fine so bring up the main window */
                    Cell_window_pop_up(cell_interface->window);
                  }
                  else
                  {
                    display_message(ERROR_MESSAGE,"CREATE(Cell_interface).  "
                      "Unable to create the CMGUI interface object");
                    DESTROY(Cell_interface)(&cell_interface);
                    cell_interface = (struct Cell_interface *)NULL;
                  }
                }
                else
                {
                  display_message(ERROR_MESSAGE,"CREATE(Cell_interface).  "
                    "Unable to create the UnEMAP interface object");
                  DESTROY(Cell_interface)(&cell_interface);
                  cell_interface = (struct Cell_interface *)NULL;
                }
              }
              else
              {
                display_message(ERROR_MESSAGE,"CREATE(Cell_interface).  "
                  "Unable to create the cell graphic list");
                DESTROY(Cell_interface)(&cell_interface);
                cell_interface = (struct Cell_interface *)NULL;
              }
            }
            else
            {
              display_message(ERROR_MESSAGE,"CREATE(Cell_interface).  "
                "Unable to create the cell variable list");
              DESTROY(Cell_interface)(&cell_interface);
              cell_interface = (struct Cell_interface *)NULL;
            }
          }
          else
          {
            display_message(ERROR_MESSAGE,"CREATE(Cell_interface).  "
              "Unable to create the cell component list");
            DESTROY(Cell_interface)(&cell_interface);
            cell_interface = (struct Cell_interface *)NULL;
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"CREATE(Cell_interface).  "
            "Unable to create the cell input object");
          DESTROY(Cell_interface)(&cell_interface);
          cell_interface = (struct Cell_interface *)NULL;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"CREATE(Cell_interface).  "
          "Unable to create the cell window object");
        DESTROY(Cell_interface)(&cell_interface);
        cell_interface = (struct Cell_interface *)NULL;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"CREATE(Cell_interface).  "
        "Unable to allocate memory for the Cell interface object");
      cell_interface = (struct Cell_interface *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Cell_interface).  "
      "Invalid argument(s)");
    cell_interface = (struct Cell_interface *)NULL;
  }
  LEAVE;
  return(cell_interface);
} /* CREATE(Cell_interface) */

int DESTROY(Cell_interface)(struct Cell_interface **cell_interface_address)
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Destroys the Cell_interface object.
==============================================================================*/
{
	int return_code;
  struct Cell_interface *cell_interface;

	ENTER(DESTROY(Cell_interface));
	if (cell_interface_address && (cell_interface = *cell_interface_address))
	{
    if (cell_interface->time_keeper)
    {
      DEACCESS(Time_keeper)(&(cell_interface->time_keeper));
    }
    if (cell_interface->window)
    {
      DESTROY(Cell_window)(&(cell_interface->window));
    }
    if (cell_interface->input != (struct Cell_input *)NULL)
    {
      DESTROY(Cell_input)(&(cell_interface->input));
    }
    if (cell_interface->calculate != (struct Cell_calculate *)NULL)
    {
      DESTROY(Cell_calculate)(&(cell_interface->calculate));
    }
    if (cell_interface->variable_editing_dialog !=
      (struct Cell_variable_editing_dialog *)NULL)
    {
      DESTROY(Cell_variable_editing_dialog)(
        &(cell_interface->variable_editing_dialog));
    }
    if (cell_interface->unemap_interface !=
      (struct Cell_unemap_interface *)NULL)
    {
      DESTROY(Cell_unemap_interface)(&(cell_interface->unemap_interface));
    }
    if (cell_interface->cmgui_interface !=
      (struct Cell_cmgui_interface *)NULL)
    {
      DESTROY(Cell_cmgui_interface)(&(cell_interface->cmgui_interface));
    }
#if defined (CELL_DISTRIBUTED)
    if (cell_interface->distributed_editing_interface !=
      (struct Distributed_editing_interface *)NULL)
    {
      DESTROY(Distributed_editing_interface)(
        &(cell_interface->distributed_editing_interface));
    }
#endif /* defined (CELL_DISTRIBUTED) */
#if defined (CELL_DISTRIBUTED)
    if (cell_interface->export_dialog != (struct Cell_export_dialog *)NULL)
    {
      DESTROY(Cell_export_dialog)(&(cell_interface->export_dialog));
    }
#endif /* defined (CELL_DISTRIBUTED) */
    /* Destroy the variable lists in each of the components */
    FOR_EACH_OBJECT_IN_LIST(Cell_component)(
      Cell_component_destroy_variable_list,NULL,cell_interface->component_list);
    /* And destroy the two main lists!! */
    if (cell_interface->component_list)
    {
      /* Removing the objects from the list will cause them to be destroy'ed */
      REMOVE_ALL_OBJECTS_FROM_LIST(Cell_component)
        (cell_interface->component_list);
      /* and destroy the list */
      DESTROY(LIST(Cell_component))(&(cell_interface->component_list));
    }
    if (cell_interface->variable_list)
    {
      /* Removing the objects from the list will cause them to be destroy'ed */
      REMOVE_ALL_OBJECTS_FROM_LIST(Cell_variable)
        (cell_interface->variable_list);
      /* and destroy the list */
      DESTROY(LIST(Cell_variable))(&(cell_interface->variable_list));
    }
    /* Clear the scene */
    if (cell_interface->cmgui_interface)
    {
      Cell_cmgui_interface_clear_scene(cell_interface->cmgui_interface,
        (void *)(cell_interface->graphic_list));
    }
    if (cell_interface->graphic_list)
    {
      /* Removing the objects from the list will cause them to be destroy'ed */
      REMOVE_ALL_OBJECTS_FROM_LIST(Cell_graphic)
        (cell_interface->graphic_list);
      /* and destroy the list */
      DESTROY(LIST(Cell_graphic))(&(cell_interface->graphic_list));
    }
		DEALLOCATE(*cell_interface_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Cell_interface).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* DESTROY(Cell_interface) */

int Cell_interface_close_model(struct Cell_interface *cell_interface)
/*******************************************************************************
LAST MODIFIED : 19 October 2000

DESCRIPTION :
Destroys all the current components and variables in the interface.
==============================================================================*/
{
	int return_code;

	ENTER(Cell_interface_close_model);
	if (cell_interface)
	{
    /* Can throw away the calculation object */
    if (cell_interface->calculate != (struct Cell_calculate *)NULL)
    {
      DESTROY(Cell_calculate)(&(cell_interface->calculate));
    }
    /* Throw away an existing unemap interface object */
    if (cell_interface->unemap_interface !=
      (struct Cell_unemap_interface *)NULL)
    {
      Cell_unemap_interface_close(cell_interface->unemap_interface);
    }
    /* Can't simply destroy the input object, just "close" it */
    if (cell_interface->input != (struct Cell_input *)NULL)
    {
      Cell_input_close(cell_interface->input);
    }
    /* Destroy the variable lists in each of the components */
    FOR_EACH_OBJECT_IN_LIST(Cell_component)(
      Cell_component_destroy_variable_list,NULL,cell_interface->component_list);
    /* And destroy the two main lists!! */
    if (cell_interface->component_list)
    {
      /* Removing the objects from the list will cause them to be destroy'ed */
      REMOVE_ALL_OBJECTS_FROM_LIST(Cell_component)
        (cell_interface->component_list);
    }
    if (cell_interface->variable_list)
    {
      /* Removing the objects from the list will cause them to be destroy'ed */
      REMOVE_ALL_OBJECTS_FROM_LIST(Cell_variable)
        (cell_interface->variable_list);
    }
    if (cell_interface->cmgui_interface)
    {
      Cell_cmgui_interface_clear_scene(cell_interface->cmgui_interface,
        (void *)(cell_interface->graphic_list));
    }
    if (cell_interface->graphic_list)
    {
      /* Removing the objects from the list will cause them to be destroy'ed */
      REMOVE_ALL_OBJECTS_FROM_LIST(Cell_graphic)
        (cell_interface->graphic_list);
    }
    /* Re-set the main window title */
    Cell_window_set_title_bar(cell_interface->window,"Cell");
    return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cell_interface_close_model.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* Cell_interface_close_model() */

void Cell_interface_terminate_XMLParser(struct Cell_interface *cell_interface)
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Terminates the XML Parser. Need to do this so that the parser is only ever
initialised once and terminated once for the enitre life of this application
instance. Required because the Xerces library can only be initialised and
terminated once!!
==============================================================================*/
{
  ENTER(Cell_interface_terminate_XMLParser);
  if (cell_interface->XMLParser_initialised)
  {
    XMLParser_terminate();
  }
  LEAVE;
} /* Cell_interface_terminate_XMLParser() */

int Cell_interface_read_model(struct Cell_interface *cell_interface,
  char *filename)
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
Reads in a Cell model. If no <filename> is given, then prompts for a file name.
==============================================================================*/
{
  int return_code = 0;
  char *display_name,*title;

  ENTER(Cell_interface_read_model);
  if (cell_interface)
  {
    if (filename != (char *)NULL)
    {
      /* destroy any existing model */
      if (Cell_interface_close_model(cell_interface))
      {
        /* ??? Need to make sure that we have a calculate object to
         * ??? pass through
         */
        if (cell_interface->calculate == (struct Cell_calculate *)NULL)
        {
          cell_interface->calculate = CREATE(Cell_calculate)(
            cell_interface->unemap_interface);
        }
        if (cell_interface->calculate)
        {
          display_name = Cell_input_read_model(cell_interface->input,filename,
            &(cell_interface->XMLParser_initialised),
            cell_interface->component_list,cell_interface->variable_list,
            cell_interface->graphic_list,cell_interface->calculate,
            cell_interface->cmgui_interface);
          /* set the main window title bar */
          if (display_name)
          {
            if (ALLOCATE(title,char,strlen("Cell - ")+strlen(display_name)+1))
            {
              sprintf(title,"Cell - %s",display_name);
              Cell_window_set_title_bar(cell_interface->window,title);
            }
            else
            {
              Cell_window_set_title_bar(cell_interface->window,display_name);
            }
          }
          else
          {
            Cell_window_set_title_bar(cell_interface->window,"Cell - ??");
          }
          /* Draw the Cell scene */
          return_code = Cell_cmgui_interface_draw_component_graphics(
            cell_interface->cmgui_interface,
            (void *)(cell_interface->component_list));
        }
        else
        {
          display_message(ERROR_MESSAGE,"Cell_interface_read_model. "
            "Unable to create the cell calculate object");
          return_code = 0;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"Cell_interface_read_model. "
          "Unable to clear existing model");
        return_code = 0;
      }
    }
    else
    {
      display_message(INFORMATION_MESSAGE,
        "No file name given\n");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_read_model.  "
      "Missing Cell interface object");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_interface_read_model() */

int Cell_interface_write_model(struct Cell_interface *cell_interface,
  char *filename)
/*******************************************************************************
LAST MODIFIED : 13 November 2000

DESCRIPTION :
Writes out a Cell model.
==============================================================================*/
{
  int return_code = 0;
  struct Cell_output *cell_output;

  ENTER(Cell_interface_write_model);
  if (cell_interface)
  {
    if (filename != (char *)NULL)
    {
      /* Create the cell output object */
      if (cell_output = CREATE(Cell_output)(cell_interface->input,
        cell_interface->calculate))
      {
        return_code = Cell_output_write_model_to_file(cell_output,filename,
          cell_interface->variable_list);
        DESTROY(Cell_output)(&cell_output);
      }
      else
      {
        display_message(ERROR_MESSAGE,"Cell_interface_write_model.  "
          "Unable to create the cell output object");
        return_code = 0;
      }
    }
    else
    {
      display_message(INFORMATION_MESSAGE,"Cell_interface_write_model.  "
        "No file name given\n");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_write_model.  "
      "Missing Cell interface object");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_interface_write_model() */

int Cell_interface_write_model_to_ipcell_file(
  struct Cell_interface *cell_interface,char *filename)
/*******************************************************************************
LAST MODIFIED : 11 January 2001

DESCRIPTION :
Writes out a Cell model to a ipcell file
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_interface_write_model_to_ipcell_file);
  if (cell_interface)
  {
    if (filename != (char *)NULL)
    {
      return_code = Cell_output_write_model_to_ipcell_file(filename,
        cell_interface->variable_list);
    }
    else
    {
      display_message(INFORMATION_MESSAGE,
        "Cell_interface_write_model_to_ipcell_file.  "
        "No file name given\n");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_write_model_to_ipcell_file.  "
      "Missing Cell interface object");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_interface_write_model_to_ipcell_file() */

int Cell_interface_list_components(
  struct Cell_interface *cell_interface,int full)
/*******************************************************************************
LAST MODIFIED : 09 July 2000

DESCRIPTION :
Lists out the current set of cell components. If <full> is not 0, then a full
listing is given, otherwise simply gives a list of names.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_interface_list_components);
  if (cell_interface)
  {
    return_code = FOR_EACH_OBJECT_IN_LIST(Cell_component)(
      Cell_component_list,(void *)(&full),cell_interface->component_list);
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_list_components.  "
      "Missing Cell interface object");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_interface_list_components() */

int Cell_interface_list_XMLParser_properties(
  struct Cell_interface *cell_interface)
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Lists out the current set of XML parser properties
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_interface_list_XMLParser_properties);
  if (cell_interface)
  {
    Cell_input_list_XMLParser_properties(cell_interface->input,
      cell_interface->XMLParser_initialised);
    return_code = 1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_list_XMLParser_properties.  "
      "Missing Cell interface object");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_interface_list_XMLParser_properties() */

int *Cell_interface_get_XMLParser_properties(
  struct Cell_interface *cell_interface)
/*******************************************************************************
LAST MODIFIED : 02 July 2000

DESCRIPTION :
Gets the current set of XML parser properties
==============================================================================*/
{
  int *properties;
  
  ENTER(Cell_interface_get_XMLParser_properties);
  if (cell_interface)
  {
    properties =
      Cell_input_get_XMLParser_properties(cell_interface->input);
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_get_XMLParser_properties.  "
      "Missing Cell interface object");
    properties = (int *)NULL;
  }
  LEAVE;
  return(properties);
} /* Cell_interface_get_XMLParser_properties() */

int Cell_interface_set_XMLParser_properties(
  struct Cell_interface *cell_interface,int *properties)
/*******************************************************************************
LAST MODIFIED : 02 July 2000

DESCRIPTION :
Sets the current set of XML parser properties
==============================================================================*/
{
  int return_code = 0;
  
  ENTER(Cell_interface_set_XMLParser_properties);
  if (cell_interface && properties)
  {
    return_code =
      Cell_input_set_XMLParser_properties(cell_interface->input,
        properties);
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_set_XMLParser_properties.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_interface_set_XMLParser_properties() */

char **Cell_interface_get_copy_tags(
  struct Cell_interface *cell_interface)
/*******************************************************************************
LAST MODIFIED : 09 July 2000

DESCRIPTION :
Gets a copy of the current set of copy tags.
==============================================================================*/
{
  char **copy_tags;
  
  ENTER(Cell_interface_get_copy_tags);
  if (cell_interface)
  {
    copy_tags =
      Cell_input_get_copy_tags(cell_interface->input);
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_get_copy_tags.  "
      "Missing Cell interface object");
    copy_tags = (char **)NULL;
  }
  LEAVE;
  return(copy_tags);
} /* Cell_interface_get_copy_tags() */

int Cell_interface_set_copy_tags(
  struct Cell_interface *cell_interface,char **copy_tags)
/*******************************************************************************
LAST MODIFIED : 09 July 2000

DESCRIPTION :
Sets the current set of copy tags.
==============================================================================*/
{
  int return_code = 0;
  
  ENTER(Cell_interface_set_copy_tags);
  if (cell_interface && copy_tags)
  {
    while(copy_tags[return_code])
    {
      display_message(INFORMATION_MESSAGE,"COPY_TAG: \"%s\"\n",
        copy_tags[return_code]);
      return_code++;
    }
    /*return_code =
      Cell_input_set_copy_tags(cell_interface->input,copy_tags);*/
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_set_copy_tags.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_interface_set_copy_tags() */

int Cell_interface_list_copy_tags(struct Cell_interface *cell_interface)
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Lists out the current set of copy tags
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_interface_list_copy_tags);
  if (cell_interface)
  {
    Cell_input_list_copy_tags(cell_interface->input);
    return_code = 1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_list_copy_tags.  "
      "Missing Cell interface object");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_interface_list_copy_tags() */

int Cell_interface_list_ref_tags(struct Cell_interface *cell_interface)
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Lists out the current set of ref tags
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_interface_list_ref_tags);
  if (cell_interface)
  {
    Cell_input_list_ref_tags(cell_interface->input);
    return_code = 1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_list_ref_tags.  "
      "Missing Cell interface object");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_interface_list_ref_tags() */

int Cell_interface_list_hierarchy(struct Cell_interface *cell_interface,
  int full,char *name)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Lists out the hierarchy described by the component with the given <name>, or
the root level component if no name is given.
==============================================================================*/
{
  int return_code = 0;
  struct Cell_component *cell_component = (struct Cell_component *)NULL;
  int indent_level = 0;

  ENTER(Cell_interface_list_hierarchy);
  USE_PARAMETER(full);
  if (cell_interface)
  {
    /* get the appropriate component */
    if (name)
    {
      cell_component = FIND_BY_IDENTIFIER_IN_LIST(Cell_component,name)(name,
        cell_interface->component_list);
    }
    else
    {
      cell_component = FIND_BY_IDENTIFIER_IN_LIST(Cell_component,name)(
        ROOT_ELEMENT_ID,cell_interface->component_list);
    }
    if (cell_component)
    {
      Cell_component_list_component_hierarchy(cell_component,
        (void *)(&indent_level));
      return_code = 1;
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_interface_list_hierarchy.  "
        "Unable to get the cell component");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_list_hierarchy.  "
      "Missing Cell interface object");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_interface_list_hierarchy() */

int Cell_interface_calculate_model(struct Cell_interface *cell_interface)
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Calculates the current cell model.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_interface_calculate_model);
  if (cell_interface)
  {
    if (cell_interface->calculate)
    {
      return_code = Cell_calculate_calculate_model(cell_interface->calculate,
        cell_interface->variable_list);
    }
    else
    {
      display_message(INFORMATION_MESSAGE,"Cell_interface_calculate_model.  "
        "Nothing to calculate\n");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_calculate_model.  "
      "Missing Cell interface object");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_interface_calculate_model() */

int Cell_interface_pop_up_calculate_dialog(
  struct Cell_interface *cell_interface)
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Brings up the calculate dialog.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_interface_pop_up_calculate_dialog);
  if (cell_interface)
  {
    if (cell_interface->calculate)
    {
      return_code = Cell_calculate_pop_up_dialog(cell_interface->calculate,
        Cell_window_get_shell(cell_interface->window),cell_interface,
        cell_interface->user_interface);
    }
    else
    {
      display_message(INFORMATION_MESSAGE,
        "Cell_interface_pop_up_calculate_dialog.  "
        "Nothing to calculate\n");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_pop_up_calculate_dialog.  "
      "Missing Cell interface object");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_interface_pop_up_calculate_dialog() */

int Cell_interface_pop_up_unemap(struct Cell_interface *cell_interface)
/*******************************************************************************
LAST MODIFIED : 01 November 2000

DESCRIPTION :
Pops up the UnEmap windows.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_interface_pop_up_unemap);
  if (cell_interface)
  {
    if (cell_interface->unemap_interface)
    {
      return_code = Cell_unemap_interface_pop_up_analysis_window(
        cell_interface->unemap_interface);
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_interface_pop_up_unemap.  "
        "Missing UnEmap interface object");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_pop_up_unemap.  "
      "Missing Cell interface object");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_interface_pop_up_unemap() */

int Cell_interface_clear_unemap(struct Cell_interface *cell_interface)
/*******************************************************************************
LAST MODIFIED : 05 November 2000

DESCRIPTION :
Clears the UnEmap windows.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_interface_clear_unemap);
  if (cell_interface)
  {
    if (cell_interface->unemap_interface)
    {
      return_code = Cell_unemap_interface_clear_analysis_work_area(
        cell_interface->unemap_interface);
    }
    else
    {
      return_code = 1;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_clear_unemap.  "
      "Missing Cell interface object");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_interface_clear_unemap() */

int Cell_interface_set_save_signals(struct Cell_interface *cell_interface,
  int save)
/*******************************************************************************
LAST MODIFIED : 05 November 2000

DESCRIPTION :
Sets the value of the save signals toggle in the UnEmap interface object.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_interface_set_save_signals);
  if (cell_interface)
  {
    if (cell_interface->unemap_interface)
    {
      return_code = Cell_unemap_interface_set_save_signals(
        cell_interface->unemap_interface,save);
    }
    else
    {
      return_code = 1;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_set_save_signals.  "
      "Missing Cell interface object");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_interface_set_save_signals() */

int Cell_interface_list_root_component(struct Cell_interface *cell_interface)
/*******************************************************************************
LAST MODIFIED : 06 November 2000

DESCRIPTION :
Lists out the root component from the cell interface.
==============================================================================*/
{
  int return_code = 0,full = 1;
  struct Cell_component *root_component;

  ENTER(Cell_interface_list_root_component);
  if (cell_interface)
  {
    if (root_component = FIND_BY_IDENTIFIER_IN_LIST(Cell_component,name)(
      ROOT_ELEMENT_ID,cell_interface->component_list))
    {
      return_code = Cell_component_list(root_component,
        (void *)(&full));
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_interface_list_root_component.  "
        "Unable to find the root component");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_list_root_component.  "
      "Missing Cell interface object");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_interface_list_root_component() */

int Cell_interface_edit_component_variables(
  struct Cell_interface *cell_interface,char *name,int reset)
/*******************************************************************************
LAST MODIFIED : 07 November 2000

DESCRIPTION :
Pops up the cell variable editing dialog with the variables from the component
given by <name>. If <name> is NULL, then the root component is used. <reset>
specifies whether the variable editing dialog is cleared before adding the
component <name> to the dialog.
==============================================================================*/
{
  int return_code = 0;
  struct Cell_component *cell_component;

  ENTER(Cell_interface_edit_component_variables);
  if (cell_interface)
  {
    if (!(cell_interface->variable_editing_dialog))
    {
      cell_interface->variable_editing_dialog =
        CREATE(Cell_variable_editing_dialog)(cell_interface,
          Cell_window_get_root_shell(cell_interface->window),
          cell_interface->user_interface);
    }
    if (cell_interface->variable_editing_dialog)
    {
      /* Get the required component */
      if (name)
      {
        cell_component = FIND_BY_IDENTIFIER_IN_LIST(Cell_component,name)(name,
          cell_interface->component_list);
      }
      else
      {
        cell_component = FIND_BY_IDENTIFIER_IN_LIST(Cell_component,name)(
          ROOT_ELEMENT_ID,cell_interface->component_list);
      }
      if (cell_component)
      {
        /* If required, clear all existing components from the dialog */
        if (reset)
        {
          Cell_variable_editing_dialog_clear_dialog(
            cell_interface->variable_editing_dialog);
        }
        /* Add the specified component to the dialog */
        return_code = Cell_variable_editing_dialog_add_component(
          cell_interface->variable_editing_dialog,cell_component);
        if (return_code)
        {
          /* Make sure that the dialog is "popped-up" */
          Cell_variable_editing_dialog_pop_up(
            cell_interface->variable_editing_dialog);
        }
        else
        {
          display_message(ERROR_MESSAGE,
            "Cell_interface_edit_component_variables.  "
            "Unable to add the component to the variable editing dialog");
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "Cell_interface_edit_component_variables.  "
          "Unable to get the cell component");
        return_code = 0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_interface_edit_component_variables.  "
        "Missing variable editing dialog");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_edit_component_variables.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_interface_edit_component_variables() */

int Cell_interface_pop_up(struct Cell_interface *cell_interface)
/*******************************************************************************
LAST MODIFIED : 07 November 2000

DESCRIPTION :
Pops up the cell interface
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_interface_pop_up);
  if (cell_interface && cell_interface->window)
  {
    Cell_window_pop_up(cell_interface->window);
    return_code = 1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_pop_up.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_interface_pop_up() */

#if defined (CELL_DISTRIBUTED)
int Cell_interface_pop_up_distributed_editing_dialog(
  struct Cell_interface *cell_interface,Widget activation)
/*******************************************************************************
LAST MODIFIED : 12 January 2001

DESCRIPTION :
Pops up the distributed editing dialog. <activation> is the toggle button
widget which activated the dialog.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_interface_pop_up_distributed_editing_dialog);
  if (cell_interface)
  {
    if ((cell_interface->distributed_editing_interface !=
      (struct Distributed_editing_interface *)NULL) ||
      (cell_interface->distributed_editing_interface =
        CREATE(Distributed_editing_interface)(cell_interface,
          cell_interface->cmgui_interface)))
    {
      return_code = Distributed_editing_interface_pop_up_dialog(
        cell_interface->distributed_editing_interface,
        Cell_window_get_shell(cell_interface->window),
        cell_interface->user_interface,activation);
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "Cell_interface_pop_up_distributed_editing_dialog.  "
        "Missing distributed editing interface object");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_interface_pop_up_distributed_editing_dialog.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_interface_pop_up_distributed_editing_dialog() */
#endif /* defined (CELL_DISTRIBUTED) */

#if defined (CELL_DISTRIBUTED)
int Cell_interface_destroy_distributed_editing_dialog(
  struct Cell_interface *cell_interface)
/*******************************************************************************
LAST MODIFIED : 12 January 2001

DESCRIPTION :
Destroys the distributed editing dialog.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_interface_destroy_distributed_editing_dialog);
  if (cell_interface)
  {
    if (cell_interface->distributed_editing_interface)
    {
      return_code = DESTROY(Distributed_editing_interface)(
        &(cell_interface->distributed_editing_interface));
    }
    else
    {
      return_code = 1;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_interface_destroy_distributed_editing_dialog.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_interface_destroy_distributed_editing_dialog() */
#endif /* defined (CELL_DISTRIBUTED) */

struct LIST(Cell_variable) *Cell_interface_get_variable_list(
  struct Cell_interface *cell_interface)
/*******************************************************************************
LAST MODIFIED : 17 January 2001

DESCRIPTION :
Returns the <cell_interface>'s variable list.
==============================================================================*/
{
  struct LIST(Cell_variable) *variable_list;

  ENTER(Cell_interface_get_variable_list);
  if (cell_interface)
  {
    variable_list = cell_interface->variable_list;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_get_variable_list.  "
      "Missing Cell interface object");
    variable_list = (struct LIST(Cell_variable) *)NULL;
  }
  LEAVE;
  return(variable_list);
} /* Cell_interface_get_variable_list() */

int Cell_interface_close(struct Cell_interface *cell_interface)
/*******************************************************************************
LAST MODIFIED : 22 January 2001

DESCRIPTION :
Closes all the Cell interface's.
==============================================================================*/
{
	int return_code;

	ENTER(Cell_interface_close);
	if (cell_interface)
	{
    if (cell_interface->calculate != (struct Cell_calculate *)NULL)
    {
      Cell_calculate_pop_down_dialog(cell_interface->calculate);
    }
    if (cell_interface->variable_editing_dialog !=
      (struct Cell_variable_editing_dialog *)NULL)
    {
      Cell_variable_editing_dialog_pop_down(
        cell_interface->variable_editing_dialog);
    }
    if (cell_interface->unemap_interface !=
      (struct Cell_unemap_interface *)NULL)
    {
      Cell_unemap_interface_pop_down_analysis_window(
        cell_interface->unemap_interface);
    }
#if defined (CELL_DISTRIBUTED)
    if (cell_interface->distributed_editing_interface !=
      (struct Distributed_editing_interface *)NULL)
    {
      Distributed_editing_interface_pop_down_dialog(
        cell_interface->distributed_editing_interface);
    }
    if (cell_interface->export_dialog != (struct Cell_export_dialog *)NULL)
    {
      Cell_export_dialog_pop_down(cell_interface->export_dialog);
    }
#endif /* defined (CELL_DISTRIBUTED) */
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cell_interface_close.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	return(return_code);
} /* Cell_interface_close() */

#if defined (CELL_DISTRIBUTED)
int Cell_interface_pop_up_export_dialog(struct Cell_interface *cell_interface)
/*******************************************************************************
LAST MODIFIED : 31 January 2001

DESCRIPTION :
Brings up the export (to CMISS) dialog.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_interface_pop_up_export_dialog);
  if (cell_interface)
  {
    if (!cell_interface->export_dialog)
    {
      if (!(cell_interface->export_dialog = CREATE(Cell_export_dialog)(
        cell_interface,cell_interface->cmgui_interface,
        cell_interface->distributed_editing_interface,
        cell_interface->user_interface,
        Cell_window_get_shell(cell_interface->window))))
      {
        display_message(ERROR_MESSAGE,"Cell_interface_pop_up_export_dialog.  "
          "Unable to create the export dialog");
      }
    }
    if (cell_interface->export_dialog)
    {
      return_code = Cell_export_dialog_pop_up(cell_interface->export_dialog);
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_interface_pop_up_export_dialog.  "
        "Missing export dialog");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_pop_up_export_dialog.  "
      "Missing Cell interface object");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_interface_pop_up_export_dialog() */
#endif /* defined (CELL_DISTRIBUTED) */

#if defined (CELL_DISTRIBUTED)
int Cell_interface_export_to_ipcell(struct Cell_interface *cell_interface,
  char *filename)
/*******************************************************************************
LAST MODIFIED : 02 February 2001

DESCRIPTION :
Exports an IPCELL file
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_interface_export_to_ipcell);
  if (cell_interface && filename)
  {
    return_code = Cell_output_write_model_to_ipcell_file_from_fields(filename,
      cell_interface->variable_list,cell_interface->cmgui_interface);
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_export_to_ipcell.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_interface_export_to_ipcell() */
#endif /* defined (CELL_DISTRIBUTED) */

#if defined (CELL_DISTRIBUTED)
int Cell_interface_export_to_ipmatc(struct Cell_interface *cell_interface,
  char *filename,void *element_group_void,void *grid_field_void)
/*******************************************************************************
LAST MODIFIED : 02 February 2001

DESCRIPTION :
Exports an IPMATC file
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_interface_export_to_ipmatc);
  if (cell_interface && filename && element_group_void && grid_field_void)
  {
    return_code = Cell_output_write_model_to_ipmatc_file_from_fields(filename,
      element_group_void,grid_field_void,cell_interface->cmgui_interface,
      cell_interface->variable_list);
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_export_to_ipmatc.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_interface_export_to_ipmatc() */
#endif /* defined (CELL_DISTRIBUTED) */

int Cell_interface_list_variables(struct Cell_interface *cell_interface,
  char *component_name,char *variable_name,int full)
/*******************************************************************************
LAST MODIFIED : 03 April 2001

DESCRIPTION :
If a <component_name> and a <variable_name> are specified, lists out the
variable if it is found in the component, else if only a <component_name> is
given, lists out all the variables in the component. If neither a
<component_name> or a <variable_name> is specified, lists out all the variables
in the model. If only a <variable_name> is specified, looks for that variable
in the main variable list and if found lists it out.
==============================================================================*/
{
  struct Cell_component *cell_component;
  struct Cell_variable *cell_variable;
  struct LIST(Cell_variable) *variable_list;
  int return_code;

  ENTER(Cell_interface_list_variable_value);
  if (cell_interface)
  {
    /* get the appropriate component */
    if (component_name)
    {
      if (cell_component = FIND_BY_IDENTIFIER_IN_LIST(Cell_component,name)(
        component_name,cell_interface->component_list))
      {
        variable_list = Cell_component_get_variable_list(cell_component);
      }
      else
      {
        variable_list = (struct LIST(Cell_variable) *)NULL;
      }
    }
    else
    {
      cell_component = (struct Cell_component *)NULL;
      variable_list = cell_interface->variable_list;
    }
    if (cell_component && variable_name)
    {
      if (cell_variable = Cell_component_get_cell_variable_by_name(
        cell_component,variable_name))
      {
        return_code = Cell_variable_list(cell_variable,(void *)(&full));
      }
      else
      {
        display_message(ERROR_MESSAGE,"Cell_interface_list_variables.  "
          "Unable to find the variable \"%s\" in the component \"%s\"",
          variable_name,component_name);
        return_code = 0;
      }
    }
    else if (variable_list && variable_name)
    {
      if (cell_variable = FIND_BY_IDENTIFIER_IN_LIST(Cell_variable,name)(
        variable_name,variable_list))
      {
        return_code = Cell_variable_list(cell_variable,(void *)(&full));
      }
      else
      {
        display_message(ERROR_MESSAGE,"Cell_interface_list_variables.  "
          "Unable to find the variable \"%s\"",variable_name);
        return_code = 0;
      }
    }
    else if (variable_list)
    {
      return_code = FOR_EACH_OBJECT_IN_LIST(Cell_variable)(
        Cell_variable_list,(void *)(&full),variable_list);
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_interface_list_variables.  "
        "Unable to get the cell component");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_list_variables.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_interface_list_variable() */

int Cell_interface_set_variable_value_from_string(
  struct Cell_interface *cell_interface,char *component_name,
  char *variable_name,char *value_string)
/*******************************************************************************
LAST MODIFIED : 03 April 2001

DESCRIPTION :
If a <component_name> and a <variable_name> are specified and the given variable
is found in the variable list for the given component, the variable's value is
set to the given <value_string>. If no <component_name> is specified, the root
component is assumed.
==============================================================================*/
{
  struct Cell_component *cell_component;
  struct Cell_variable *cell_variable;
  int return_code;

  ENTER(Cell_interface_list_variable_value);
  if (cell_interface && variable_name)
  {
    /* get the appropriate component */
    if (component_name)
    {
      cell_component = FIND_BY_IDENTIFIER_IN_LIST(Cell_component,name)(
        component_name,cell_interface->component_list);
    }
    else
    {
      cell_component = FIND_BY_IDENTIFIER_IN_LIST(Cell_component,name)(
        ROOT_ELEMENT_ID,cell_interface->component_list);
    }
    if (cell_component)
    {
      if (cell_variable = Cell_component_get_cell_variable_by_name(
        cell_component,variable_name))
      {
        if (return_code = Cell_variable_set_value_from_string(cell_variable,
          value_string))
        {
          /* Set the changed flag for the variable */
          Cell_variable_set_changed(cell_variable,1);
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"Cell_interface_set_variable_value.  "
          "Unable to find the variable \"%s\" in the component \"%s\"",
          variable_name,component_name ? component_name : ROOT_ELEMENT_ID);
        return_code = 0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_interface_set_variable_value.  "
        "Unable to get the cell component");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_set_variable_value.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_interface_set_variable_value() */

int Cell_interface_set_calculate(struct Cell_interface *cell_interface,
  float Tstart,float Tend,float dT,float tabT,char *model_routine_name,
  char *model_dso_name,char *intg_routine_name,char *intg_dso_name,
  char *data_file_name)
/*******************************************************************************
LAST MODIFIED : 04 April 2001

DESCRIPTION :
Sets the values of the <cell_interface>'s calculate objec to those specified.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_interface_set_calculate);
  if (cell_interface && model_routine_name && intg_routine_name &&
    (Tstart <= Tend) && (dT > 0) && (tabT > 0))
  {
    if (cell_interface->calculate)
    {
      if (Cell_calculate_set_model_routine_name(cell_interface->calculate,
        model_routine_name) &&
        Cell_calculate_set_dso_name(cell_interface->calculate,model_dso_name) &&
        Cell_calculate_set_intg_routine_name(cell_interface->calculate,
          intg_routine_name) &&
        Cell_calculate_set_intg_dso_name(cell_interface->calculate,
          intg_dso_name) &&
        Cell_calculate_set_data_file_name(cell_interface->calculate,
          data_file_name) &&
        Cell_calculate_set_start_time(cell_interface->calculate,Tstart) &&
        Cell_calculate_set_end_time(cell_interface->calculate,Tend) &&
        Cell_calculate_set_dt(cell_interface->calculate,dT) &&
        Cell_calculate_set_tabt(cell_interface->calculate,tabT))
      {
        return_code = 1;
      }
      else
      {
        display_message(ERROR_MESSAGE,"Cell_interface_set_calculate.  "
          "Unable to set all parameters");
        return_code = 0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_interface_set_calculate.  "
        "Missing cell calculate object");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_set_calculate.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_interface_set_calculate() */

int Cell_interface_list_calculate(struct Cell_interface *cell_interface)
/*******************************************************************************
LAST MODIFIED : 04 April 2001

DESCRIPTION :
Lists out the current calculate object.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_interface_list_calculate);
  if (cell_interface)
  {
    if (cell_interface->calculate)
    {
      return_code = Cell_calculate_list(cell_interface->calculate);
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_interface_list_calculate.  "
        "Missing cell calculate object");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_list_calculate.  "
      "Missing Cell interface object");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_interface_list_calculate() */

float Cell_interface_get_start_time(struct Cell_interface *cell_interface)
/*******************************************************************************
LAST MODIFIED : 04 April 2001

DESCRIPTION :
Returns the integration start time as a float.
==============================================================================*/
{
  float start_time;

  ENTER(Cell_interface_get_start_time);
  if (cell_interface)
  {
    if (cell_interface->calculate)
    {
      start_time = Cell_calculate_get_start_time(cell_interface->calculate);
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_interface_get_start_time.  "
        "Missing calculation object");
      start_time = 0.0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_get_start_time.  "
      "Invalid argument(s)");
    start_time = 0.0;
  }
  LEAVE;
  return(start_time);
} /* Cell_interface_get_start_time() */

float Cell_interface_get_end_time(struct Cell_interface *cell_interface)
/*******************************************************************************
LAST MODIFIED : 04 April 2001

DESCRIPTION :
Returns the integration end time as a float.
==============================================================================*/
{
  float end_time;

  ENTER(Cell_interface_get_end_time);
  if (cell_interface)
  {
    if (cell_interface->calculate)
    {
      end_time = Cell_calculate_get_end_time(cell_interface->calculate);
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_interface_get_end_time.  "
        "Missing calculation object");
      end_time = 0.0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_get_end_time.  "
      "Invalid argument(s)");
    end_time = 0.0;
  }
  LEAVE;
  return(end_time);
} /* Cell_interface_get_end_time() */

float Cell_interface_get_dt(struct Cell_interface *cell_interface)
/*******************************************************************************
LAST MODIFIED : 04 April 2001

DESCRIPTION :
Returns the integration time step as a float.
==============================================================================*/
{
  float dt;

  ENTER(Cell_interface_get_dt);
  if (cell_interface)
  {
    if (cell_interface->calculate)
    {
      dt = Cell_calculate_get_dt(cell_interface->calculate);
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_interface_get_dt.  "
        "Missing calculation object");
      dt = 0.0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_get_dt.  "
      "Invalid argument(s)");
    dt = 0.0;
  }
  LEAVE;
  return(dt);
} /* Cell_interface_get_dt() */

float Cell_interface_get_tabt(struct Cell_interface *cell_interface)
/*******************************************************************************
LAST MODIFIED : 04 April 2001

DESCRIPTION :
Returns the integration tabulation interval as a float.
==============================================================================*/
{
  float tabt;

  ENTER(Cell_interface_get_tabt);
  if (cell_interface)
  {
    if (cell_interface->calculate)
    {
      tabt = Cell_calculate_get_tabt(cell_interface->calculate);
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_interface_get_tabt.  "
        "Missing calculation object");
      tabt = 0.0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_get_tabt.  "
      "Invalid argument(s)");
    tabt = 0.0;
  }
  LEAVE;
  return(tabt);
} /* Cell_interface_get_tabt() */

char *Cell_interface_get_model_routine_name(
  struct Cell_interface *cell_interface)
/*******************************************************************************
LAST MODIFIED : 04 April 2001

DESCRIPTION :
Returns the model routine name.
==============================================================================*/
{
  char *name;

  ENTER(Cell_interface_get_model_routine_name);
  if (cell_interface)
  {
    if (cell_interface->calculate)
    {
      name = Cell_calculate_get_model_routine_name(cell_interface->calculate);
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_interface_get_model_routine_name.  "
        "Missing calculation object");
      name = (char *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_get_model_routine_name.  "
      "Invalid argument(s)");
    name = (char *)NULL;
  }
  LEAVE;
  return(name);
} /* Cell_interface_get_model_routine_name() */

char *Cell_interface_get_model_dso_name(struct Cell_interface *cell_interface)
/*******************************************************************************
LAST MODIFIED : 04 April 2001

DESCRIPTION :
Returns the model DSO name.
==============================================================================*/
{
  char *name;

  ENTER(Cell_interface_get_model_dso_name);
  if (cell_interface)
  {
    if (cell_interface->calculate)
    {
      name = Cell_calculate_get_dso_name(cell_interface->calculate);
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_interface_get_model_dso_name.  "
        "Missing calculation object");
      name = (char *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_get_model_dso_name.  "
      "Invalid argument(s)");
    name = (char *)NULL;
  }
  LEAVE;
  return(name);
} /* Cell_interface_get_model_dso_name() */

char *Cell_interface_get_intg_routine_name(
  struct Cell_interface *cell_interface)
/*******************************************************************************
LAST MODIFIED : 04 April 2001

DESCRIPTION :
Returns the integrator routine name.
==============================================================================*/
{
  char *name;

  ENTER(Cell_interface_get_intg_routine_name);
  if (cell_interface)
  {
    if (cell_interface->calculate)
    {
      name = Cell_calculate_get_intg_routine_name(cell_interface->calculate);
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_interface_get_intg_routine_name.  "
        "Missing calculation object");
      name = (char *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_get_intg_routine_name.  "
      "Invalid argument(s)");
    name = (char *)NULL;
  }
  LEAVE;
  return(name);
} /* Cell_interface_get_intg_routine_name() */

char *Cell_interface_get_intg_dso_name(struct Cell_interface *cell_interface)
/*******************************************************************************
LAST MODIFIED : 04 April 2001

DESCRIPTION :
Returns the integrator DSO name.
==============================================================================*/
{
  char *name;

  ENTER(Cell_interface_get_intg_dso_name);
  if (cell_interface)
  {
    if (cell_interface->calculate)
    {
      name = Cell_calculate_get_intg_dso_name(cell_interface->calculate);
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_interface_get_intg_dso_name.  "
        "Missing calculation object");
      name = (char *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_get_intg_dso_name.  "
      "Invalid argument(s)");
    name = (char *)NULL;
  }
  LEAVE;
  return(name);
} /* Cell_interface_get_intg_dso_name() */

char *Cell_interface_get_data_file_name(struct Cell_interface *cell_interface)
/*******************************************************************************
LAST MODIFIED : 04 April 2001

DESCRIPTION :
Returns the data file name.
==============================================================================*/
{
  char *name;

  ENTER(Cell_interface_get_data_file_name);
  if (cell_interface)
  {
    if (cell_interface->calculate)
    {
      name = Cell_calculate_get_data_file_name(cell_interface->calculate);
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_interface_get_data_file_name.  "
        "Missing calculation object");
      name = (char *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_interface_get_data_file_name.  "
      "Invalid argument(s)");
    name = (char *)NULL;
  }
  LEAVE;
  return(name);
} /* Cell_interface_get_data_file_name() */

