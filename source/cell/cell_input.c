/*******************************************************************************
FILE : cell_input.c

LAST MODIFIED : 15 March 2001

DESCRIPTION :
Input routines for the cell interface.
==============================================================================*/

#include <stdio.h>
#include <string.h>

/* The XML header files */
#include "local_utils/crim.h"
#include "local_utils/xerces_wrapper.h"
#include "local_utils/xpath_wrapper.h"

#include "cell/cell_input.h"
#include "command/parser.h"

/*
Module objects
--------------
*/
struct Cell_input
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
A data object used to input cell models into the Cell interface data objects.
==============================================================================*/
{
  /* the XML features that we want to control, add as required */
  int do_validation; /* determines whether validation is performed */
  int do_namespaces; /*     "        "     name space processing is enabled */
  int do_expand;     /*     "        "     entity references are expanded */
  /* pointer to the CRIM parsed document */
  void *crim_document;
  /* all the copy elements to look for */
  char **copy_tags;
  /* all the reference elements to look for */
  char **ref_tags;
}; /* struct Cell_input */

/*
Module functions
----------------
*/
void create_Cell_unit_abbreviation_table(void *parent)
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
Creates a look-up table for all the unit abbreviations stored in the <parent>
element of the DOM tree.
==============================================================================*/
{
  int position;
  char path[256];
  void *abbreviation;

  position = 1;
  sprintf(path,"child::units[position()=%d]",position);
  while (abbreviation = XPath_evaluate(parent,path))
  {
    display_message(INFORMATION_MESSAGE,
      "Found abbreviation, \"%s\" => \"%s\"\n",
      XMLParser_get_attribute_value(abbreviation,"abbreviation"),
      XMLParser_get_attribute_value(abbreviation,"expanded"));
    position++;
    sprintf(path,"child::units[position()=%d]",position);
  } /* while (abbreviation = XPath_evaluate(parent,path)) */
} /* create_Cell_unit_abbreviation_table() */

static int create_Cell_variables_from_dom(void *parent,
  struct Cell_component *cell_component,
  struct LIST(Cell_variable) *variable_list)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
Creates all the variable objects found as children of the given <parent>
element in the DOM tree.
==============================================================================*/
{
  int return_code = 0;
  int position;
  char path[256];
  void *variable;
  struct Cell_variable *cell_variable;

  ENTER(create_Cell_variables_from_dom);
  if (parent && cell_component && variable_list)
  {
    position = 1;
    sprintf(path,"child::declares_variable[position()=%d]",position);
    return_code = 1;
    while (return_code && (variable = XPath_evaluate(parent,path)))
    {
      /* Create the cell variable object and add it to the global variable list,
       * and add the variable to the component's variable list.
       */
      if ((cell_variable = CREATE(Cell_variable)(
        XMLParser_get_attribute_value(variable,"name"))) &&
        ADD_OBJECT_TO_LIST(Cell_variable)(cell_variable,variable_list) &&
        Cell_component_add_variable_to_variable_list(cell_component,
          cell_variable))
      {
        Cell_variable_set_display_name(cell_variable,
          XMLParser_get_attribute_value(variable,"display_name"));
        if (!(Cell_variable_set_value_type(cell_variable,CELL_REAL_VALUE) &&
          Cell_variable_set_value_from_string(cell_variable,
            XMLParser_get_attribute_value(variable,"value"))))
        {
          display_message(WARNING_MESSAGE,"create_Cell_variables_from_dom. "
            "Unable to set the value for variable: %s",
            XMLParser_get_attribute_value(variable,"name"));
        }
        position++;
        sprintf(path,"child::declares_variable[position()=%d]",position);
      }
      else
      {
        display_message(ERROR_MESSAGE,"create_Cell_variables_from_dom.  "
          "Unable to create the variable and add it to the variable list");
        return_code = 0;
      }
    } /* while (return_code && (variable = XPath_evaluate(parent,path))) */
  }
  else
  {
    display_message(ERROR_MESSAGE,"create_Cell_variables_from_dom.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* create_Cell_variables_from_dom() */

#if defined (OLD_CODE)

static int create_exported_Cell_variables_from_dom(void *parent,
  struct Cell_component *cell_component)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Creates all the exported variable objects found as children of the given
<parent> element in the DOM tree.
==============================================================================*/
{
  int return_code = 0;
  int position;
  char path[256];
  void *variable;
  struct Cell_variable *cell_variable;

  ENTER(create_exported_Cell_variables_from_dom);
  if (parent && cell_component)
  {
    position = 1;
    sprintf(path,"child::export_variable[position()=%d]",position);
    return_code = 1;
    while (return_code && (variable = XPath_evaluate(parent,path)))
    {
      /* Create the cell variable object and add the variable to the
       * component's exported variable list.
       */
      if ((cell_variable = CREATE(Cell_variable)(
        XMLParser_get_attribute_value(variable,"name_ref"))) &&
        Cell_component_add_variable_to_exported_variable_list(cell_component,
          cell_variable))
      {
        position++;
        sprintf(path,"child::export_variable[position()=%d]",position);
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "create_exported_Cell_variables_from_dom.  "
          "Unable to create the variable and add it to the variable list");
        return_code = 0;
      }
    } /* while (return_code && (variable = XPath_evaluate(parent,path))) */
  }
  else
  {
    display_message(ERROR_MESSAGE,"create_exported_Cell_variables_from_dom.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* create_exported_Cell_variables_from_dom() */

static int create_imported_Cell_variables_from_dom(void *parent,
  struct Cell_component *cell_component)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Creates all the imported variable objects found as children of the given
<parent> element in the DOM tree.
==============================================================================*/
{
  int return_code = 0;
  int position;
  char path[256];
  void *variable;
  struct Cell_variable *cell_variable;

  ENTER(create_imported_Cell_variables_from_dom);
  if (parent && cell_component)
  {
    position = 1;
    sprintf(path,"child::import_variable[position()=%d]",position);
    return_code = 1;
    while (return_code && (variable = XPath_evaluate(parent,path)))
    {
      /* Create the cell variable object and add the variable to the
       * component's imported variable list.
       */
      if ((cell_variable = CREATE(Cell_variable)(
        XMLParser_get_attribute_value(variable,"name_ref"))) &&
        Cell_component_add_variable_to_imported_variable_list(cell_component,
          cell_variable))
      {
        position++;
        sprintf(path,"child::import_variable[position()=%d]",position);
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "create_imported_Cell_variables_from_dom.  "
          "Unable to create the variable and add it to the variable list");
        return_code = 0;
      }
    } /* while (return_code && (variable = XPath_evaluate(parent,path))) */
  }
  else
  {
    display_message(ERROR_MESSAGE,"create_imported_Cell_variables_from_dom.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* create_imported_Cell_variables_from_dom() */

static int create_Cell_mechanisms_from_dom(void *parent,
  struct Cell_component *parent_component,
  struct LIST(Cell_component) *component_list,
  struct LIST(Cell_variable) *variable_list)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
Creates all the mechanism objects found as children of the given <parent>
element in the DOM tree.
==============================================================================*/
{
  int return_code = 0;
  int position;
  char path[256];
  void *mechanism;
  struct Cell_component *cell_component;

  ENTER(create_Cell_mechanisms_from_dom);
  if (parent && variable_list)
  {
    position = 1;
    sprintf(path,"child::mechanism[position()=%d]",position);
    return_code = 1;
    while (return_code && (mechanism = XPath_evaluate(parent,path)))
    {
      /* display_message(INFORMATION_MESSAGE,
        "Found mechanism: \"%s\"\n",
        XMLParser_get_attribute_value(mechanism,"name"));*/
      if ((cell_component = CREATE(Cell_component)(parent_component,
        XMLParser_get_attribute_value(mechanism,"name"))) &&
        ADD_OBJECT_TO_LIST(Cell_component)(cell_component,component_list))
      {
/*          Cell_component_set_component_type_mechanism(cell_component); */
        Cell_component_set_display_name(cell_component,
          XMLParser_get_attribute_value(mechanism,"display_name"));
        if (create_Cell_variables_from_dom(mechanism,cell_component,
          variable_list))
        {
          if (create_imported_Cell_variables_from_dom(mechanism,cell_component))
          {
            position++;
            sprintf(path,"child::mechanism[position()=%d]",position);
          }
          else
          {
            display_message(ERROR_MESSAGE,"create_Cell_mechanisms_from_dom.  "
              "Unable to create the mechanism's imported variables");
            return_code = 0;
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"create_Cell_mechanisms_from_dom.  "
            "Unable to create the mechanism's variables");
          return_code = 0;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"create_Cell_mechanisms_from_dom.  "
          "Unable to create the mechanism and add it to the component list");
        return_code = 0;
      }
    } /* while (return_code && (mechanism = XPath_evaluate(parent,path))) */
  }
  else
  {
    display_message(ERROR_MESSAGE,"create_Cell_mechanisms_from_dom.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* create_Cell_mechanisms_from_dom() */

static int create_Cell_boundaries_from_dom(void *parent,
  struct Cell_component *parent_component,
  struct LIST(Cell_component) *component_list,
  struct LIST(Cell_variable) *variable_list)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
Creates all the boundary objects found as children of the given <parent>
element in the DOM tree.
==============================================================================*/
{
  int return_code = 0;
  int position,reference_counter;
  char path[256];
  void *boundary,*between,*uses,*subspace_ref;
  struct Cell_component *cell_component;
  struct Cell_component *subspace_ref_component;

  ENTER(create_Cell_boundaries_from_dom);
  if (parent && component_list && variable_list)
  {
    position = 1;
    sprintf(path,"child::boundary[position()=%d]",position);
    return_code = 1;
    while (return_code && (boundary = XPath_evaluate(parent,path)))
    {
      if ((cell_component = CREATE(Cell_component)(parent_component,
        XMLParser_get_attribute_value(boundary,"name"))) &&
        ADD_OBJECT_TO_LIST(Cell_component)(cell_component,component_list))
      {
/*          Cell_component_set_component_type_boundary(cell_component); */
        Cell_component_set_display_name(cell_component,
          XMLParser_get_attribute_value(boundary,"display_name"));
        /* The boundary is only useful if it has both <between> and <uses>
         * elements!!
         */
        sprintf(path,"child::between");
        if (between = XPath_evaluate(boundary,path))
        {
          sprintf(path,"child::uses");
          if (uses = XPath_evaluate(boundary,path))
          {
            /* Get the subspaces the boundary is between */
            reference_counter = 1;
            sprintf(path,"child::subspace_ref[position()=%d]",
              reference_counter);
            while (return_code && (subspace_ref = XPath_evaluate(between,path)))
            {
              if (subspace_ref_component =
                CREATE(Cell_component)(cell_component,
                  XMLParser_get_attribute_value(subspace_ref,"name_ref")))
              {
/*                  Cell_component_set_component_type_subspace_ref( */
/*                    subspace_ref_component); */
                /* check for varaibles exported to the boundary scope */
                if (create_exported_Cell_variables_from_dom(subspace_ref,
                  subspace_ref_component))
                {
                  reference_counter++;
                  sprintf(path,"child::subspace_ref[position()=%d]",
                    reference_counter);
                }
                else
                {
                  display_message(ERROR_MESSAGE,
                    "create_Cell_boundaries_from_dom.  "
                    "Unable to create exported variables");
                  return_code = 0;
                }
              }
              else
              {
                display_message(ERROR_MESSAGE,
                  "create_Cell_boundaries_from_dom.  "
                  "Unable to create the subspace reference and add it to the "
                  "component list");
                return_code = 0;
              }
            } /* while (return_code &&
                 (subspace_ref = XPath_evaluate(between,path))) */
            /* create the mechanisms used */
            create_Cell_mechanisms_from_dom(uses,cell_component,component_list,
              variable_list);
          } /* if (uses = XPath_evaluate(boundary,path)) */
        } /* if (between = XPath_evaluate(boundary,path)) */
        position++;
        sprintf(path,"child::boundary[position()=%d]",position);
      }
      else
      {
        display_message(ERROR_MESSAGE,"create_Cell_boundaries_from_dom.  "
          "Unable to create the boundary and add it to the component list");
        return_code = 0;
      }
    } /* while (return_code && (boundary = XPath_evaluate(parent,path))) */
  }
  else
  {
    display_message(ERROR_MESSAGE,"create_Cell_boundaries_from_dom.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* create_Cell_boundaries_from_dom() */

#endif /* defined (OLD_CODE) */

static int create_Cell_components_from_dom(void *parent,
  struct Cell_component *parent_component,
  struct LIST(Cell_component) *component_list,
  struct LIST(Cell_variable) *variable_list)
/*******************************************************************************
LAST MODIFIED : 17 October 2000

DESCRIPTION :
Creates all the component objects found as children of the given <parent>
element in the DOM tree.

??? What to do with boundaries ???
==============================================================================*/
{
  int return_code = 0;
  int position;
  char path[256];
  void *component;
  struct Cell_component *cell_component;

  ENTER(create_Cell_components_from_dom);
  if (parent && component_list && variable_list)
  {
    position = 1;
    sprintf(path,"child::component[position()=%d]",position);
    return_code = 1;
    /* get all the <component> children of the parent node */
    while (return_code && (component = XPath_evaluate(parent,path)))
    {
      /*display_message(INFORMATION_MESSAGE,
        "Found subspace: \"%s\"\n",
        XMLParser_get_attribute_value(subspace,"name"));*/
      if ((cell_component = CREATE(Cell_component)(parent_component,
        XMLParser_get_attribute_value(component,"name"))) &&
        ADD_OBJECT_TO_LIST(Cell_component)(cell_component,component_list))
      {
        Cell_component_set_display_name(cell_component,
          XMLParser_get_attribute_value(component,"display_name"));
        if (create_Cell_variables_from_dom(component,cell_component,
          variable_list))
        {
          /* the recursive bit .. */
          if (create_Cell_components_from_dom(component,cell_component,
            component_list,variable_list))
          {
            position++;
            sprintf(path,"child::component[position()=%d]",position);
          }
          else
          {
            display_message(ERROR_MESSAGE,"create_Cell_components_from_dom.  "
              "Unable to create the component's child components");
            return_code = 0;
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"create_Cell_components_from_dom.  "
            "Unable to create the components's variables");
          return_code = 0;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"create_Cell_components_from_dom.  "
          "Unable to create the component and add it to the component list");
        return_code = 0;
      }
    } /* while (return_code && (component = XPath_evaluate(parent,path))) */
  }
  else
  {
    display_message(ERROR_MESSAGE,"create_Cell_components_from_dom.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* create_Cell_components_from_dom() */

static int build_cmiss_interface_information(void *parent,
  struct LIST(Cell_component) *component_list)
/*******************************************************************************
LAST MODIFIED : 25 October 2000

DESCRIPTION :
Adds specified CMISS interface information to variables
==============================================================================*/
{
  int return_code = 0;
  int position;
  char path[256],*name,*string;
  void *map_variable;
  struct Cell_component *cell_component;
  struct Cell_variable *cell_variable;

  ENTER(build_cmiss_interface_information);
  if (parent && component_list)
  {
    position = 1;
    sprintf(path,"child::map_variable[position()=%d]",position);
    return_code = 1;
    /* get all the <map_variable> children of the parent node */
    while (return_code && (map_variable = XPath_evaluate(parent,path)))
    {
      if (name = XMLParser_get_attribute_value(map_variable,"component_ref"))
      {
        cell_component =
          FIND_BY_IDENTIFIER_IN_LIST(Cell_component,name)(name,component_list);
        DEALLOCATE(name);
      }
      else
      {
        cell_component =
          FIND_BY_IDENTIFIER_IN_LIST(Cell_component,name)(ROOT_ELEMENT_ID,
            component_list);
      }
      if (cell_component)
      {
        if (name = XMLParser_get_attribute_value(map_variable,"name_ref"))
        {
          if (cell_variable =
            Cell_component_get_cell_variable_by_name(cell_component,name))
          {
            Cell_variable_set_cmiss_interface(cell_variable,
              XMLParser_get_attribute_value(map_variable,"array"),
              XMLParser_get_attribute_value(map_variable,"position"));
            if (string = XMLParser_get_attribute_value(map_variable,"plot"))
            {
              if (fuzzy_string_compare_same_length(string,"unemap"))
              {
                Cell_variable_set_unemap_interface(cell_variable);
              }
              DEALLOCATE(string);
            }
            if (string = XMLParser_get_attribute_value(map_variable,"ode"))
            {
              if (fuzzy_string_compare_same_length(string,"true"))
              {
                Cell_variable_set_ode(cell_variable);
              }
              DEALLOCATE(string);
            }
          }
          else
          {
            display_message(ERROR_MESSAGE,"build_cmiss_interface_information.  "
              "Unable to find the coresponding cell variable object");
          }
          DEALLOCATE(name);
        }
        else
        {
          display_message(ERROR_MESSAGE,"build_cmiss_interface_information.  "
            "Unable to find the name_ref for the variable map");
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"build_cmiss_interface_information.  "
          "Unable to find the correct cell component");
      }
      position++;
      sprintf(path,"child::map_variable[position()=%d]",position);
    } /* while (return_code && (map_variable = XPath_evaluate(parent,path))) */
  }
  else
  {
    display_message(ERROR_MESSAGE,"build_cmiss_interface_information.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* build_cmiss_interface_information() */

static int build_calculation_information(void *parent,
  struct Cell_calculate *cell_calculate)
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
Sets information in the <cell_calculate> object.
==============================================================================*/
{
  int return_code = 0;
  char *value_string;

  ENTER(build_calculation_information);
  if (parent && cell_calculate)
  {
    /* Set the model routine name */
    if (Cell_calculate_set_model_routine_name(cell_calculate,
      XMLParser_get_attribute_value(parent,"model_routine_name")))
    {
      /* Set the DSO file name */
      if (Cell_calculate_set_dso_name(cell_calculate,
        XMLParser_get_attribute_value(parent,"model_dso_name")))
      {
        /* Set the integrator routine name */
        if (Cell_calculate_set_intg_routine_name(cell_calculate,
          XMLParser_get_attribute_value(parent,"integrator_routine_name")))
        {
          /* Set the integrator DSO file name */
          if (Cell_calculate_set_intg_dso_name(cell_calculate,
            XMLParser_get_attribute_value(parent,"integrator_dso_name")))
          {
            /* Set the time integration parameters */
            if (value_string = XMLParser_get_attribute_value(parent,"tstart"))
            {
              Cell_calculate_set_start_time_from_string(cell_calculate,
                value_string);
            }
            if (value_string = XMLParser_get_attribute_value(parent,"tend"))
            {
              Cell_calculate_set_end_time_from_string(cell_calculate,
                value_string);
            }
            if (value_string = XMLParser_get_attribute_value(parent,"dt"))
            {
              Cell_calculate_set_dt_from_string(cell_calculate,
                value_string);
            }
            if (value_string = XMLParser_get_attribute_value(parent,"tabt"))
            {
              Cell_calculate_set_tabt_from_string(cell_calculate,
                value_string);
            }
          
          }
          else
          {
            display_message(ERROR_MESSAGE,"build_calculation_information.  "
              "Unable to set the integrator DSO name");
            return_code = 0;
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"build_calculation_information.  "
            "Unable to set the integrator routine name");
          return_code = 0;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"build_calculation_information.  "
          "Unable to set the model DSO name");
        return_code = 0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"build_calculation_information.  "
        "Unable to set the model routine name");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"build_calculation_information.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* build_calculation_information() */

static int modify_variables(void *parent,
  struct LIST(Cell_variable) *variable_list)
/*******************************************************************************
LAST MODIFIED : 10 November 2000

DESCRIPTION :
Modifies the values for the variables given in the varaible modification
element in the Cell configuration file.
==============================================================================*/
{
  int return_code = 0,position;
  char path[256],*name_ref,*value_string;
  void *modify_variable;
  struct Cell_variable *variable;

  ENTER(modify_variables);
  if (parent && variable_list &&
    (NUMBER_IN_LIST(Cell_variable)(variable_list) > 0))
  {
    position = 1;
    sprintf(path,"child::modify_variable[position()=%d]",position);
    return_code = 1;
    /* Get all the <modify_variable> children of the parent node */
    while (return_code && (modify_variable = XPath_evaluate(parent,path)))
    {
      /* Get the name_ref attribute */
      if (name_ref =
        XMLParser_get_attribute_value(modify_variable,"name_ref"))
      {
        /* Find the corresponding variable */
        if (variable = FIND_BY_IDENTIFIER_IN_LIST(Cell_variable,name)(
          name_ref,variable_list))
        {
          /* Get the value string */
          if (value_string =
            XMLParser_get_attribute_value(modify_variable,"value"))
          {
            /* Set the variable's value */
            if (Cell_variable_set_value_from_string(variable,value_string))
            {
              Cell_variable_set_changed(variable,1);
            }
            else
            {
              display_message(WARNING_MESSAGE,"modify_variables.  "
                "The value \"%s\" is invalid for the variable \"%s\"",
                value_string,name_ref);
            }
            DEALLOCATE(value_string);
          }
          else
          {
            display_message(WARNING_MESSAGE,"modify_variables.  "
              "No value given for the variable with the name: \"%s\"",name_ref);
          }
        }
        else
        {
          display_message(WARNING_MESSAGE,"modify_variables.  "
            "No variable found with the name: \"%s\"",name_ref);
        }
        DEALLOCATE(name_ref);
      }
      else
      {
        /* Just ignore this element */
        display_message(WARNING_MESSAGE,"modify_variables.  "
          "Missing name_ref attribute");
      }
      position++;
      sprintf(path,"child::modify_variable[position()=%d]",position);
    } /* while (return_code && (modify_variable = ...)) */
  }
  else
  {
    display_message(ERROR_MESSAGE,"modify_variables.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* modify_variables() */

static int build_graphics(void *parent,
  struct Cell_cmgui_interface *cmgui_interface,
  struct LIST(Cell_graphic) *graphic_list)
/*******************************************************************************
LAST MODIFIED : 18 November 2000

DESCRIPTION :
Uses the information contained in the <parent> DOM element to create any
graphics specified in the config file.
==============================================================================*/
{
  int return_code = 0,position;
  char path[256],path2[256],path3[256],path4[256],*name;
  void *graphic,*diffuse,*ambient,*emission,*specular;
  struct Cell_graphic *cell_graphic;

  ENTER(build_graphics);
  if (parent && graphic_list)
  {
    /* Loop through all the graphic elements */
    position = 1;
    sprintf(path,"child::graphic[position()=%d]",position);
    return_code = 1;
    while (return_code && (graphic = XPath_evaluate(parent,path)))
    {
      /* Create the graphic */
      if (name = XMLParser_get_attribute_value(graphic,"name"))
      {
        if ((cell_graphic = CREATE(Cell_graphic)(name)) &&
          ADD_OBJECT_TO_LIST(Cell_graphic)(cell_graphic,graphic_list))
        {
          /* Create the graphic's graphical object */
          Cell_graphic_create_graphical_object(cell_graphic,cmgui_interface,
            XMLParser_get_attribute_value(graphic,"obj_file"));
          /* Create the graphic's and graphical material */
          /* ???? DPN - do you need to really have all 4 of these ???
           */
          sprintf(path,"child::diffuse");
          sprintf(path2,"child::ambient");
          sprintf(path3,"child::emission");
          sprintf(path4,"child::specular");
          if ((diffuse = XPath_evaluate(graphic,path)) &&
            (ambient = XPath_evaluate(graphic,path2)) &&
            (emission = XPath_evaluate(graphic,path3)) &&
            (specular = XPath_evaluate(graphic,path4)))
          {
            Cell_graphic_create_graphical_material(cell_graphic,cmgui_interface,
              XMLParser_get_attribute_value(diffuse,"red"),
              XMLParser_get_attribute_value(diffuse,"green"),
              XMLParser_get_attribute_value(diffuse,"blue"),
              XMLParser_get_attribute_value(ambient,"red"),
              XMLParser_get_attribute_value(ambient,"green"),
              XMLParser_get_attribute_value(ambient,"blue"),
              XMLParser_get_attribute_value(emission,"red"),
              XMLParser_get_attribute_value(emission,"green"),
              XMLParser_get_attribute_value(emission,"blue"),
              XMLParser_get_attribute_value(specular,"red"),
              XMLParser_get_attribute_value(specular,"green"),
              XMLParser_get_attribute_value(specular,"blue"),
              XMLParser_get_attribute_value(graphic,"alpha"),
              XMLParser_get_attribute_value(graphic,"shininess"));
          }
          else
          {
            display_message(WARNING_MESSAGE,"build_graphics.  "
              "Unable to create the material for the \"%s\" graphic",name);
          }
        }
        else
        {
          display_message(WARNING_MESSAGE,"build_graphics.  "
            "Unable to create the \"%s\" graphic and add it to the list",name);
          if (cell_graphic)
          {
            DESTROY(Cell_graphic)(&cell_graphic);
          }
        }
        DEALLOCATE(name);
      }
      else
      {
        display_message(WARNING_MESSAGE,"build_graphics.  "
          "Unable to get the graphic's name");
      }
      position++;
      sprintf(path,"child::graphic[position()=%d]",position);
    } /* while (return_code && (graphic = XPath_evaluate(parent,path))) */
  }
  else
  {
    display_message(ERROR_MESSAGE,"build_graphics.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* build_graphics() */

static int build_graphical_display(void *parent,
  struct Cell_cmgui_interface *cmgui_interface,
  struct LIST(Cell_component) *component_list,
  struct LIST(Cell_graphic) *graphic_list)
/*******************************************************************************
LAST MODIFIED : 18 November 2000

DESCRIPTION :
Uses the information contained in the <parent> DOM element to set-up the
graphical representation of CellML components.
==============================================================================*/
{
  int return_code = 0,dom_position;
  char path[256],path2[256],path3[256],*name;
  void *display,*element,*position,*direction,*scale;
  struct Cell_component *component;
  struct Cell_graphic *graphic;

  ENTER(build_graphical_display);
  if (parent && component_list)
  {
    /* Loop through all the display elements */
    dom_position = 1;
    sprintf(path,"child::display[position()=%d]",dom_position);
    return_code = 1;
    while (return_code && (display = XPath_evaluate(parent,path)))
    {
      /* Get the component referenced */
      sprintf(path,"child::component_ref");
      if (element = XPath_evaluate(display,path))
      {
        if (name = XMLParser_get_attribute_value(element,"name_ref"))
        {
          component = FIND_BY_IDENTIFIER_IN_LIST(Cell_component,name)(
            name,component_list);
          DEALLOCATE(name);
          sprintf(path,"child::position");
          sprintf(path2,"child::direction");
          sprintf(path3,"child::scale");
          if ((position = XPath_evaluate(display,path)) &&
            (direction = XPath_evaluate(display,path2)) &&
            (scale = XPath_evaluate(display,path3)))
          {
            /* Set-up the graphical transformation matrix */
            if (Cell_component_set_graphical_transformation(component,
              XMLParser_get_attribute_value(position,"x"),
              XMLParser_get_attribute_value(position,"y"),
              XMLParser_get_attribute_value(position,"z"),
              XMLParser_get_attribute_value(direction,"x"),
              XMLParser_get_attribute_value(direction,"y"),
              XMLParser_get_attribute_value(direction,"z"),
              XMLParser_get_attribute_value(scale,"x"),
              XMLParser_get_attribute_value(scale,"y"),
              XMLParser_get_attribute_value(scale,"z")))
            {
              /* Now need to associate the component with its graphic */
              sprintf(path,"child::graphic_ref");
              if (element = XPath_evaluate(display,path))
              {
                if (name = XMLParser_get_attribute_value(element,"name_ref"))
                {
                  graphic = FIND_BY_IDENTIFIER_IN_LIST(Cell_graphic,name)(
                    name,graphic_list);
                  Cell_component_set_graphic(component,graphic);
                  DEALLOCATE(name);
                }
              }
              else
              {
                /* No component reference found, look for a straight graphic
                   definition */
                sprintf(path,"child::graphic");
                if (element = XPath_evaluate(display,path))
                {
                  /* build the graphic */
                  build_graphics(display,cmgui_interface,graphic_list);
                  if (name = XMLParser_get_attribute_value(element,"name"))
                  {
                    graphic = FIND_BY_IDENTIFIER_IN_LIST(Cell_graphic,name)(
                      name,graphic_list);
                    Cell_component_set_graphic(component,graphic);
                    DEALLOCATE(name);
                  }
                }
                else
                {
                  /* No graphic found!! */
                  display_message(WARNING_MESSAGE,"build_graphical_display.  "
                    "No graphic found");
                }
              }
            }
            else
            {
              display_message(WARNING_MESSAGE,"build_graphical_display.  "
                "Unable to set the graphical transformation");
            }
          }
          else
          {
            display_message(WARNING_MESSAGE,"build_graphical_display.  "
              "Unable to get the graphical transformation");
          }
        }
        else
        {
          display_message(WARNING_MESSAGE,"build_graphical_display.  "
            "Unable to get a component name_ref attribute");
        }
      }
      else
      {
        display_message(WARNING_MESSAGE,"build_graphical_display.  "
          "Unable to get a component reference");
      }
      dom_position++;
      sprintf(path,"child::display[position()=%d]",dom_position);
    } /* while (return_code && (display = XPath_evaluate(parent,path))) */
  }
  else
  {
    display_message(ERROR_MESSAGE,"build_graphical_display.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* build_graphical_display() */

static void build_from_dom(void *root,
  struct LIST(Cell_component) *component_list,
  struct LIST(Cell_variable) *variable_list)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
Main routine to build the Cell data objects from the DOM tree with the given
<root> element.
==============================================================================*/
{
  char path[256];
  void *child;
  struct Cell_component *root_component;

  ENTER(build_from_dom);
  if (root && component_list && variable_list)
  {
    /* Create the units abbreviation look-up table if one is given
     */
    sprintf(path,"child::units_abbreviation_table");
    if (child = XPath_evaluate(root,path))
    {
      create_Cell_unit_abbreviation_table(child);
    }
    /* Create the root component to hold everything */
    if ((root_component = CREATE(Cell_component)((struct Cell_component *)NULL,
      ROOT_ELEMENT_ID))
      && ADD_OBJECT_TO_LIST(Cell_component)(root_component,component_list))
    {
      /* need to check for variables in the root component */
      if (create_Cell_variables_from_dom(root,root_component,
        variable_list))
      {
        if (create_Cell_components_from_dom(root,root_component,
          component_list,variable_list))
        {
        }
        else
        {
          display_message(ERROR_MESSAGE,"build_from_dom.  "
            "Unable to create the root level components");
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"build_from_dom.  "
          "Unable to create the root level variables");
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"build_from_dom.  "
        "Unable to create the root component");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"build_from_dom.  "
      "Invalid argument(s)");
  }
  LEAVE;
} /* build_from_dom() */

/*
Global functions
----------------
*/
struct Cell_input *CREATE(Cell_input)(void)
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Creates a Cell_input object.
==============================================================================*/
{
  struct Cell_input *cell_input;
  int i,allocated;
  int number_of_copy_tags = 4;
  char *default_copy_tags[4] = {
    "graphic",
    "subspace",
    "boundary",
    "mechanism"
  };
  int number_of_ref_tags = 3;
  char *default_ref_tags[3] = {
    "graphic",
    "cell_model_component",
    "subspace"
  };
  
  ENTER(CREATE(Cell_input));
  if (ALLOCATE(cell_input,struct Cell_input,1))
  {
    /* initialise the object */
    cell_input->do_validation = 0; /* turn off validation */
    cell_input->do_namespaces = 0; /* disable namespace processing */
    cell_input->do_expand     = 1; /* expand entity references */
    cell_input->crim_document = (void *)NULL;
    cell_input->copy_tags = (char **)NULL;
    cell_input->ref_tags = (char **)NULL;
    /* set the default copy tags */
    if (ALLOCATE(cell_input->copy_tags,char *,number_of_copy_tags+1))
    {
      allocated = 1;
      for (i=0;(i<number_of_copy_tags)&&allocated;i++)
      {
        if (ALLOCATE(cell_input->copy_tags[i],char,
          strlen(default_copy_tags[i])+1))
        {
          if (!strcpy(cell_input->copy_tags[i],default_copy_tags[i]))
          {
            DEALLOCATE(cell_input->copy_tags[i]);
            allocated = 0;
          }
        }
        else
        {
          allocated = 0;
        }
      }
      cell_input->copy_tags[i] = (char *)NULL;
      if (allocated)
      {
        /* set the default reference tags */
        if (ALLOCATE(cell_input->ref_tags,char *,number_of_ref_tags+1))
        {
          allocated = 1;
          for (i=0;(i<number_of_ref_tags)&&allocated;i++)
          {
            if (ALLOCATE(cell_input->ref_tags[i],char,
              strlen(default_ref_tags[i])+1))
            {
              if (!strcpy(cell_input->ref_tags[i],default_ref_tags[i]))
              {
                DEALLOCATE(cell_input->ref_tags[i]);
                allocated = 0;
              }
            }
            else
            {
              allocated = 0;
            }
          }
          cell_input->ref_tags[i] = (char *)NULL;
          if (!allocated)
          {
            display_message(ERROR_MESSAGE,"CREATE(Cell_input).  "
              "Unable to allocate memory for the individual ref tags");
            DESTROY(Cell_input)(&cell_input);
            cell_input = (struct Cell_input *)NULL;
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"CREATE(Cell_input).  "
            "Unable to allocate memory for the ref tags");
          DESTROY(Cell_input)(&cell_input);
          cell_input = (struct Cell_input *)NULL;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"CREATE(Cell_input).  "
          "Unable to allocate memory for the individual copy tags");
        DESTROY(Cell_input)(&cell_input);
        cell_input = (struct Cell_input *)NULL;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"CREATE(Cell_input).  "
        "Unable to allocate memory for the copy tags");
      DESTROY(Cell_input)(&cell_input);
      cell_input = (struct Cell_input *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Cell_input).  "
      "Unable to allocate memory for the Cell input object");
    cell_input = (struct Cell_input *)NULL;
  }
  LEAVE;
  return(cell_input);
} /* CREATE(Cell_input)() */

int DESTROY(Cell_input)(struct Cell_input **cell_input_address)
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Destroys the Cell_input object.
==============================================================================*/
{
	int return_code,i;
  struct Cell_input *cell_input;

	ENTER(DESTROY(Cell_input));
	if (cell_input_address && (cell_input = *cell_input_address))
	{
    if (cell_input->crim_document)
    {
      CRIM_destroy_document(cell_input->crim_document);
    }
    if (cell_input->copy_tags)
    {
      i = 0;
      while(cell_input->copy_tags[i])
      {
        DEALLOCATE(cell_input->copy_tags[i]);
        i++;
      }
      DEALLOCATE(cell_input->copy_tags);
    }
    if (cell_input->ref_tags)
    {
      i = 0;
      while(cell_input->ref_tags[i])
      {
        DEALLOCATE(cell_input->ref_tags[i]);
        i++;
      }
      DEALLOCATE(cell_input->ref_tags);
    }
		DEALLOCATE(*cell_input_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Cell_input).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* DESTROY(Cell_input) */

int Cell_input_close(struct Cell_input *cell_input)
/*******************************************************************************
LAST MODIFIED : 19 October 2000

DESCRIPTION :
Closes the Cell_input object.
==============================================================================*/
{
	int return_code;

	ENTER(Cell_input_close);
	if (cell_input)
	{
    if (cell_input->crim_document)
    {
      CRIM_clear_documents(cell_input->crim_document);
    }
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cell_input_close.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* Cell_input_close */

char *Cell_input_read_model(struct Cell_input *cell_input,char *filename,
  int *XMLParser_initialised,struct LIST(Cell_component) *component_list,
  struct LIST(Cell_variable) *variable_list,
  struct LIST(Cell_graphic) *graphic_list,struct Cell_calculate *cell_calculate,
  struct Cell_cmgui_interface *cmgui_interface)
/*******************************************************************************
LAST MODIFIED : 10 November 2000

DESCRIPTION :
Reads in the cell model specified in <filename>. <XMLParser_initialised> is a
pointer to the value in the Cell interface object, used to determine if the
XML parser has been initialised or not.

Returns the display name of the model.
==============================================================================*/
{
  void *cellml_element,*root_element,*element;
  char *display_name = (char *)NULL;
  
  ENTER(Cell_input_read_model);
  if (cell_input && filename && variable_list && component_list &&
    cell_calculate)
  {
    if (!(*XMLParser_initialised))
    {
      /* need to initialise the parser */
      if (XMLParser_initialise())
      {
        *XMLParser_initialised = 1;
      }
      else
      {
        display_message(ERROR_MESSAGE,"Cell_input_read_model.  "
          "Unable to initialise the XML parser");
      }
    }
    if (*XMLParser_initialised)
    {
      /* Do a CRIM parse of the model file */
      if (cell_input->crim_document = CRIMParse(filename,cell_input->copy_tags,
        cell_input->ref_tags,cell_input->do_validation,
        cell_input->do_namespaces,cell_input->do_expand))
      {
        /* Get the root element of the CellML information - could be either
         * the root element of the CellML DOM document or simply the top
         * element of the CellML description contained in the CELL input file
         */
        if (cellml_element = CRIM_get_element(cell_input->crim_document,
          "cell_model_component",/*refAllowed*/1))
        {
          /* Build the cell data objects */
          build_from_dom(cellml_element,component_list,variable_list);
          /* grab the display name of the model/experiment - if no display
           * name found, try just the name */
          if (root_element =
            CRIM_get_root_element(cell_input->crim_document,filename))
          {
            /* Get the CMISS interface information */
            if (element = CRIM_get_element(cell_input->crim_document,
              "cmiss_mapping",/*refAllowed*/0))
            {
              build_cmiss_interface_information(element,component_list);
            }
            /* Get the calculation information */
            if (element = CRIM_get_element(cell_input->crim_document,
              "model_calculation",/*refAllowed*/0))
            {
              build_calculation_information(element,cell_calculate);
            }
            /* Get any variable modification information */
            if (element = CRIM_get_element(cell_input->crim_document,
              "variable_modifications",/*refAllowed*/0))
            {
              modify_variables(element,variable_list);
            }
            /* Get any graphical display information */
            if (element = CRIM_get_element(cell_input->crim_document,
              "graphical_display",/*refAllowed*/0))
            {
              /* First need to build any "global" graphics so that they can
                 be used in the building of the graphical display */
              build_graphics(element,cmgui_interface,graphic_list);
              /* And then set-up all the component graphical representation */
              build_graphical_display(element,cmgui_interface,
                component_list,graphic_list);
            }
            if (!(display_name = XMLParser_get_attribute_value(root_element,
              "display_name")))
            {
              if (!(display_name = XMLParser_get_attribute_value(root_element,
                "name")))
              {
                if (ALLOCATE(display_name,char,strlen("Unknown Model")+1))
                {
                  sprintf(display_name,"Unknown Model");
                }
                else
                {
                  display_name = (char *)NULL;
                }
              }
            }
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"Cell_input_read_model.  "
            "Unable to get the CellML document root\n");
        }
        /*
          ??
          ?? need to keep this around for writing out again
          ??
        */
        /* tidy-up */
        /* CRIM_destroy_document(cell_input->crim_document);
         * cell_input->crim_document = (void *)NULL;
         */
      }
      else
      {
        display_message(ERROR_MESSAGE,"Cell_input_read_model.  "
          "Unable to perform a CRIM parse of the file: \"%s\"",filename);
      }
    } /* if (*XMLParser_initialised) */
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_input_read_model.  "
      "Invalid argument(s)");
  }
  LEAVE;
  return(display_name);
} /* Cell_input_read_model() */

void Cell_input_list_XMLParser_properties(struct Cell_input *cell_input,
  int XMLParser_initialised)
/*******************************************************************************
LAST MODIFIED : 30 June 2000

DESCRIPTION :
Lists out the XML parser's properties.
==============================================================================*/
{
  ENTER(Cell_input_list_XMLParser_properties);
  if (cell_input)
  {
    display_message(INFORMATION_MESSAGE,"XML Parser Properties:\n");
    if (cell_input->do_validation)
    {
      display_message(INFORMATION_MESSAGE,
        "  Validation is enabled\n");
    }
    else
    {
      display_message(INFORMATION_MESSAGE,
        "  Validation is disabled\n");
    }
    if (cell_input->do_namespaces)
    {
      display_message(INFORMATION_MESSAGE,
        "  Namespace processing is enabled\n");
    }
    else
    {
      display_message(INFORMATION_MESSAGE,
        "  Namespace processing is disabled\n");
    }
    if (cell_input->do_expand)
    {
      display_message(INFORMATION_MESSAGE,
        "  Expansion of entity references is enabled\n");
    }
    else
    {
      display_message(INFORMATION_MESSAGE,
        "  Expansion of entity references is disabled\n");
    }
    if (XMLParser_initialised)
    {
      display_message(INFORMATION_MESSAGE,
        "  The XML parser has been initialised\n");
    }
    else
    {
      display_message(INFORMATION_MESSAGE,
        "  The XML parser is not yet initialised\n");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_input_list_XMLParser_properties.  "
      "Invalid argument(s)");
  }
  LEAVE;
} /* Cell_input_list_XMLParser_properties() */

int *Cell_input_get_XMLParser_properties(struct Cell_input *cell_input)
/*******************************************************************************
LAST MODIFIED : 02 July 2000

DESCRIPTION :
Gets the XML parser's properties.
==============================================================================*/
{
  int *properties;
  
  ENTER(Cell_input_get_XMLParser_properties);
  if (cell_input)
  {
    if (ALLOCATE(properties,int,3))
    {
      properties[0] = cell_input->do_validation;
      properties[1] = cell_input->do_namespaces;
      properties[2] = cell_input->do_expand;
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_input_get_XMLParser_properties.  "
        "Unable to allocate memory for the XML parser properties");
      properties = (int *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_input_get_XMLParser_properties.  "
      "Invalid argument(s)");
    properties = (int *)NULL;
  }
  LEAVE;
  return(properties);
} /* Cell_input_get_XMLParser_properties() */

int Cell_input_set_XMLParser_properties(struct Cell_input *cell_input,
  int *properties)
/*******************************************************************************
LAST MODIFIED : 02 July 2000

DESCRIPTION :
Sets the XML parser's properties.
==============================================================================*/
{
  int return_code = 0;
  
  ENTER(Cell_input_set_XMLParser_properties);
  if (cell_input && properties)
  {
    cell_input->do_validation = properties[0];
    cell_input->do_namespaces = properties[1];
    cell_input->do_expand = properties[2];
    return_code = 1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_input_set_XMLParser_properties.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_input_set_XMLParser_properties() */

char **Cell_input_get_copy_tags(struct Cell_input *cell_input)
/*******************************************************************************
LAST MODIFIED : 09 July 2000

DESCRIPTION :
Returns a copy of the current copy tags.
==============================================================================*/
{
  char **copy_tags;
  int current_tag;
  
  ENTER(Cell_input_get_copy_tags);
  if (cell_input)
  {
    current_tag = 0;
    copy_tags = (char **)NULL;
    while((current_tag >= 0) && cell_input->copy_tags[current_tag])
    {
      if (REALLOCATE(copy_tags,copy_tags,char *,current_tag+1))
      {
        if (ALLOCATE(copy_tags[current_tag],char,
          strlen(cell_input->copy_tags[current_tag])+1))
        {
          sprintf(copy_tags[current_tag],"%s",
            cell_input->copy_tags[current_tag]);
        }
        else
        {
          display_message(ERROR_MESSAGE,"Cell_input_get_copy_tags.  "
            "Unable to allocate memory for the copy tag: \"$s\"",
            cell_input->copy_tags[current_tag]);
          current_tag = -1;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"Cell_input_get_copy_tags.  "
          "Unable to allocate memory for the copy tags");
        current_tag = -1;
      }
    } /* while(cell_input->copy_tags[current_tag]) */
    if (current_tag < 0)
    {
      current_tag = 0;
      while(copy_tags[current_tag])
      {
        DEALLOCATE(copy_tags[current_tag]);
        current_tag++;
      }
      DEALLOCATE(copy_tags);
      copy_tags = (char **)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_input_get_copy_tags.  "
      "Invalid argument(s)");
    copy_tags = (char **)NULL;
  }
  LEAVE;
  return(copy_tags);
} /* Cell_input_get_copy_tags() */

void Cell_input_list_copy_tags(struct Cell_input *cell_input)
/*******************************************************************************
LAST MODIFIED : 30 June 2000

DESCRIPTION :
Lists out the current copy tags.
==============================================================================*/
{
  int i;
  
  ENTER(Cell_input_list_copy_tags);
  if (cell_input)
  {
    if (cell_input->copy_tags)
    {
      display_message(INFORMATION_MESSAGE,
        "The current set of copy tags are:\n");
      i = 0;
      while (cell_input->copy_tags[i])
      {
        display_message(INFORMATION_MESSAGE,"  \"%s\"\n",
          cell_input->copy_tags[i]);
        i++;
      }
    }
    else
    {
      display_message(INFORMATION_MESSAGE,
        "There are no copy tags currently defined\n");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_input_list_copy_tags.  "
      "Invalid argument(s)");
  }
  LEAVE;
} /* Cell_input_list_copy_tags() */

void Cell_input_list_ref_tags(struct Cell_input *cell_input)
/*******************************************************************************
LAST MODIFIED : 30 June 2000

DESCRIPTION :
Lists out the current ref tags.
==============================================================================*/
{
  int i;
  
  ENTER(Cell_input_list_ref_tags);
  if (cell_input)
  {
    if (cell_input->ref_tags)
    {
      display_message(INFORMATION_MESSAGE,
        "The current set of reference tags are:\n");
      i = 0;
      while (cell_input->ref_tags[i])
      {
        display_message(INFORMATION_MESSAGE,"  \"%s\"\n",
          cell_input->ref_tags[i]);
        i++;
      }
    }
    else
    {
      display_message(INFORMATION_MESSAGE,
        "There are no reference tags currently defined\n");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_input_list_ref_tags.  "
      "Invalid argument(s)");
  }
  LEAVE;
} /* Cell_input_list_ref_tags() */

void *Cell_input_get_crim_document(struct Cell_input *cell_input)
/*******************************************************************************
LAST MODIFIED : 13 November 2000

DESCRIPTION :
Returns a pointer to the current CRIM document.
==============================================================================*/
{
  void *crim_document;

  ENTER(Cell_input_get_crim_document);
  if (cell_input)
  {
    crim_document = cell_input->crim_document;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_input_get_crim_document.  "
      "Invalid argument(s)");
    crim_document = (void *)NULL;
  }
  LEAVE;
  return(crim_document);
} /* Cell_input_get_crim_document() */

