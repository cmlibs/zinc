/*******************************************************************************
FILE : cell_input.c

LAST MODIFIED : 21 April 2001

DESCRIPTION :
Input routines for the cell interface.
==============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The XML header files */
#include "local_utils/crim.h"
#include "local_utils/xerces_wrapper.h"

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
LAST MODIFIED : 20 April 2001

DESCRIPTION :
Creates a look-up table for all the unit abbreviations stored in the <parent>
element of the DOM tree.
==============================================================================*/
{
  int position;
  void *abbreviation;
  char *abbr,*expand;

  position = 0;
  while (abbreviation = XMLParser_get_child_node_by_name(parent,"units",
    position))
  {
    abbr = XMLParser_get_attribute_value(abbreviation,"abbreviation");
    expand = XMLParser_get_attribute_value(abbreviation,"expanded");
    display_message(INFORMATION_MESSAGE,
      "Found abbreviation, \"%s\" => \"%s\"\n",abbr,expand);
    if (abbr) free(abbr);
    if (expand) free(expand);
    position++;
  } /* while (abbreviation) */
} /* create_Cell_unit_abbreviation_table() */

static int create_Cell_variables_from_dom(void *parent,
  struct Cell_component *cell_component,
  struct LIST(Cell_variable) *variable_list)
/*******************************************************************************
LAST MODIFIED : 20 April 2001

DESCRIPTION :
Creates all the variable objects found as children of the given <parent>
element in the DOM tree.
==============================================================================*/
{
  int return_code = 0;
  int position;
  void *variable;
  struct Cell_variable *cell_variable;
  char *name,*display_name,*value;

  ENTER(create_Cell_variables_from_dom);
  if (parent && cell_component && variable_list)
  {
    position = 0;
    return_code = 1;
    while (return_code && (variable = XMLParser_get_child_node_by_name(parent,
      "declares_variable",position)))
    {
      /* Create the cell variable object and add it to the global variable list,
       * and add the variable to the component's variable list.
       */
      name = XMLParser_get_attribute_value(variable,"name");
      if ((cell_variable = CREATE(Cell_variable)(name)) &&
        ADD_OBJECT_TO_LIST(Cell_variable)(cell_variable,variable_list) &&
        Cell_component_add_variable_to_variable_list(cell_component,
          cell_variable))
      {
        display_name = XMLParser_get_attribute_value(variable,"display_name");
        Cell_variable_set_display_name(cell_variable,display_name);
        if (display_name) free(display_name);
        value = XMLParser_get_attribute_value(variable,"value");
        if (!(Cell_variable_set_value_type(cell_variable,CELL_REAL_VALUE) &&
          Cell_variable_set_value_from_string(cell_variable,value)))
        {
          display_message(WARNING_MESSAGE,"create_Cell_variables_from_dom. "
            "Unable to set the value for variable: %s",
            XMLParser_get_attribute_value(variable,"name"));
        }
        if (value) free(value);
        position++;
      }
      else
      {
        display_message(ERROR_MESSAGE,"create_Cell_variables_from_dom.  "
          "Unable to create the variable and add it to the variable list");
        return_code = 0;
      }
      if (name) free(name);
    } /* while (return_code && variable) */
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
LAST MODIFIED : 14 April 2001

DESCRIPTION :
Creates all the exported variable objects found as children of the given
<parent> element in the DOM tree.
==============================================================================*/
{
  int return_code = 0;
  int position;
  void *variable;
  struct Cell_variable *cell_variable;

  ENTER(create_exported_Cell_variables_from_dom);
  if (parent && cell_component)
  {
    position = 0;
    return_code = 1;
    while (return_code && (variable = XMLParser_get_child_node_by_name(parent,
      "export_variables",position)))
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
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "create_exported_Cell_variables_from_dom.  "
          "Unable to create the variable and add it to the variable list");
        return_code = 0;
      }
    } /* while (return_code && variable) */
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
LAST MODIFIED : 14 April 2001

DESCRIPTION :
Creates all the imported variable objects found as children of the given
<parent> element in the DOM tree.
==============================================================================*/
{
  int return_code = 0;
  int position;
  void *variable;
  struct Cell_variable *cell_variable;

  ENTER(create_imported_Cell_variables_from_dom);
  if (parent && cell_component)
  {
    position = 0;
    return_code = 1;
    while (return_code && (variable = XMLParser_get_child_node_by_name(parent,
      "import_variable",position)))
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
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "create_imported_Cell_variables_from_dom.  "
          "Unable to create the variable and add it to the variable list");
        return_code = 0;
      }
    } /* while (return_code && variable) */
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
LAST MODIFIED : 14 April 2001

DESCRIPTION :
Creates all the mechanism objects found as children of the given <parent>
element in the DOM tree.
==============================================================================*/
{
  int return_code = 0;
  int position;
  void *mechanism;
  struct Cell_component *cell_component;

  ENTER(create_Cell_mechanisms_from_dom);
  if (parent && variable_list)
  {
    position = 0;
    return_code = 1;
    while (return_code && (mechanism = XMLParser_get_child_node_by_name(parent,
      "mechanism",position)))
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
    } /* while (return_code && mechanism) */
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
LAST MODIFIED : 14 April 2001

DESCRIPTION :
Creates all the boundary objects found as children of the given <parent>
element in the DOM tree.
==============================================================================*/
{
  int return_code = 0;
  int position,reference_counter;
  void *boundary,*between,*uses,*subspace_ref;
  struct Cell_component *cell_component;
  struct Cell_component *subspace_ref_component;

  ENTER(create_Cell_boundaries_from_dom);
  if (parent && component_list && variable_list)
  {
    position = 0;
    return_code = 1;
    while (return_code && (boundary = XMLParser_get_child_node_by_name(parent,
      "boundary",position)))
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
        if (between = XMLParser_get_child_node_by_name(boundary,"between",0))
        {
          if (uses = XMLParser_get_child_node_by_name(boundary,"uses",0))
          {
            /* Get the subspaces the boundary is between */
            reference_counter = 0;
            while (return_code && (subspace_ref =
              XMLParser_get_child_node_by_name(between,"subspace_ref",
                reference_counter)))
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
            } /* while (return_code && subspace_ref) */
            /* create the mechanisms used */
            create_Cell_mechanisms_from_dom(uses,cell_component,component_list,
              variable_list);
          } /* if (uses) */
        } /* if (between) */
        position++;
      }
      else
      {
        display_message(ERROR_MESSAGE,"create_Cell_boundaries_from_dom.  "
          "Unable to create the boundary and add it to the component list");
        return_code = 0;
      }
    } /* while (return_code && boundary) */
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
LAST MODIFIED : 20 April 2001

DESCRIPTION :
Creates all the component objects found as children of the given <parent>
element in the DOM tree.

??? What to do with boundaries ???
==============================================================================*/
{
  int return_code = 0;
  int position;
  void *component;
  struct Cell_component *cell_component;
  char *name,*display_name;

  ENTER(create_Cell_components_from_dom);
  if (parent && component_list && variable_list)
  {
    position = 0;
    return_code = 1;
    /* get all the <component> children of the parent node */
    while (return_code && (component = XMLParser_get_child_node_by_name(parent,
      "component",position)))
    {
      /*display_message(INFORMATION_MESSAGE,
        "Found subspace: \"%s\"\n",
        XMLParser_get_attribute_value(subspace,"name"));*/
      name = XMLParser_get_attribute_value(component,"name");
      if ((cell_component = CREATE(Cell_component)(parent_component,name)) &&
        ADD_OBJECT_TO_LIST(Cell_component)(cell_component,component_list))
      {
        display_name = XMLParser_get_attribute_value(component,"display_name");
        Cell_component_set_display_name(cell_component,display_name);
        if (display_name) free(display_name);
        if (create_Cell_variables_from_dom(component,cell_component,
          variable_list))
        {
          /* the recursive bit .. */
          if (create_Cell_components_from_dom(component,cell_component,
            component_list,variable_list))
          {
            position++;
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
      if (name) free(name);
    } /* while (return_code && component) */
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
LAST MODIFIED : 14 April 2001

DESCRIPTION :
Adds specified CMISS interface information to variables
==============================================================================*/
{
  int return_code = 0;
  int position;
  char *name,*string,*array,*position_string;
  void *map_variable;
  struct Cell_component *cell_component;
  struct Cell_variable *cell_variable;

  ENTER(build_cmiss_interface_information);
  if (parent && component_list)
  {
    position = 0;
    return_code = 1;
    /* get all the <map_variable> children of the parent node */
    while (return_code && (map_variable = XMLParser_get_child_node_by_name(
      parent,"map_variable",position)))
    {
      if (name = XMLParser_get_attribute_value(map_variable,"component_ref"))
      {
        cell_component =
          FIND_BY_IDENTIFIER_IN_LIST(Cell_component,name)(name,component_list);
        free(name);
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
            array = XMLParser_get_attribute_value(map_variable,"array");
            position_string = XMLParser_get_attribute_value(map_variable,
              "position");
            Cell_variable_set_cmiss_interface(cell_variable,array,
              position_string);
            if (array) free(array);
            if (position_string) free(position_string);
            if (string = XMLParser_get_attribute_value(map_variable,"plot"))
            {
              if (fuzzy_string_compare_same_length(string,"unemap"))
              {
                Cell_variable_set_unemap_interface(cell_variable);
              }
              free(string);
            }
            if (string = XMLParser_get_attribute_value(map_variable,"ode"))
            {
              if (fuzzy_string_compare_same_length(string,"true"))
              {
                Cell_variable_set_ode(cell_variable);
              }
              free(string);
            }
          }
          else
          {
            display_message(ERROR_MESSAGE,"build_cmiss_interface_information.  "
              "Unable to find the coresponding cell variable object");
          }
          free(name);
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
    } /* while (return_code && map_variable) */
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
  struct Cell_calculate *cell_calculate,char *base_path)
/*******************************************************************************
LAST MODIFIED : 20 April 2001

DESCRIPTION :
Sets information in the <cell_calculate> object.
==============================================================================*/
{
  int return_code = 0;
  char *value_string,*model_routine_name,*model_dso_name;
  char *intg_routine_name,*intg_dso_name;
  char *model_dso_path,*intg_dso_path;

  ENTER(build_calculation_information);
  if (parent && cell_calculate)
  {
    /* Set the model routine name */
    model_routine_name = XMLParser_get_attribute_value(parent,
      "model_routine_name");
    model_dso_name = XMLParser_get_attribute_value(parent,
      "model_dso_name");
    intg_routine_name = XMLParser_get_attribute_value(parent,
      "integrator_routine_name");
    intg_dso_name = XMLParser_get_attribute_value(parent,
      "integrator_dso_name");
    model_dso_path = XMLParser_get_full_path(model_dso_name,base_path);
    intg_dso_path = XMLParser_get_full_path(intg_dso_name,base_path);
    if (Cell_calculate_set_model_routine_name(cell_calculate,model_routine_name))
    {
      /* Set the DSO file name */
      if (Cell_calculate_set_dso_name(cell_calculate,model_dso_path))
      {
        /* Set the integrator routine name */
        if (Cell_calculate_set_intg_routine_name(cell_calculate,
          intg_routine_name))
        {
          /* Set the integrator DSO file name */
          if (Cell_calculate_set_intg_dso_name(cell_calculate,intg_dso_path))
          {
            /* Set the time integration parameters */
            if (value_string = XMLParser_get_attribute_value(parent,"tstart"))
            {
              Cell_calculate_set_start_time_from_string(cell_calculate,
                value_string);
              free(value_string);
            }
            if (value_string = XMLParser_get_attribute_value(parent,"tend"))
            {
              Cell_calculate_set_end_time_from_string(cell_calculate,
                value_string);
              free(value_string);
            }
            if (value_string = XMLParser_get_attribute_value(parent,"dt"))
            {
              Cell_calculate_set_dt_from_string(cell_calculate,
                value_string);
              free(value_string);
            }
            if (value_string = XMLParser_get_attribute_value(parent,"tabt"))
            {
              Cell_calculate_set_tabt_from_string(cell_calculate,
                value_string);
              free(value_string);
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
    if (model_routine_name) free(model_routine_name);
    if (model_dso_name) free(model_dso_name);
    if (intg_routine_name) free(intg_routine_name);
    if (intg_dso_name) free(intg_dso_name);
    if (model_dso_path) free(model_dso_path);
    if (intg_dso_path) free(intg_dso_path);
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
LAST MODIFIED : 20 April 2001

DESCRIPTION :
Modifies the values for the variables given in the varaible modification
element in the Cell configuration file.
==============================================================================*/
{
  int return_code = 0,position;
  char *name_ref,*value_string;
  void *modify_variable;
  struct Cell_variable *variable;

  ENTER(modify_variables);
  if (parent && variable_list &&
    (NUMBER_IN_LIST(Cell_variable)(variable_list) > 0))
  {
    position = 0;
    return_code = 1;
    /* Get all the <modify_variable> children of the parent node */
    while (return_code && (modify_variable = XMLParser_get_child_node_by_name(
      parent,"modify_variable",position)))
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
            free(value_string);
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
        free(name_ref);
      }
      else
      {
        /* Just ignore this element */
        display_message(WARNING_MESSAGE,"modify_variables.  "
          "Missing name_ref attribute");
      }
      position++;
    } /* while (return_code && modify_variable) */
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
  struct LIST(Cell_graphic) *graphic_list,char *base_path)
/*******************************************************************************
LAST MODIFIED : 20 April 2001

DESCRIPTION :
Uses the information contained in the <parent> DOM element to create any
graphics specified in the config file.
==============================================================================*/
{
  int return_code = 0,position,i;
  char *name;
  void *graphic,*diffuse,*ambient,*emission,*specular;
  struct Cell_graphic *cell_graphic;
  char *obj_file,*obj_path;
  char *dif[3],*amb[3],*emi[3],*spe[3],*alpha,*shininess;

  ENTER(build_graphics);
  if (parent && graphic_list)
  {
    /* Loop through all the graphic elements */
    position = 0;
    return_code = 1;
    while (return_code && (graphic = XMLParser_get_child_node_by_name(parent,
      "graphic",position)))
    {
      /* Create the graphic */
      if (name = XMLParser_get_attribute_value(graphic,"name"))
      {
        if ((cell_graphic = CREATE(Cell_graphic)(name)) &&
          ADD_OBJECT_TO_LIST(Cell_graphic)(cell_graphic,graphic_list))
        {
          /* Create the graphic's graphical object */
          obj_file = XMLParser_get_attribute_value(graphic,"obj_file");
          obj_path = XMLParser_get_full_path(obj_file,base_path);
          Cell_graphic_create_graphical_object(cell_graphic,cmgui_interface,
            obj_path);
          if (obj_file) free(obj_file);
          if (obj_path) free(obj_path);
          /* Create the graphic's and graphical material */
          /* ???? DPN - do you need to really have all 4 of these ???
           */
          if ((diffuse = XMLParser_get_child_node_by_name(graphic,
            "diffuse",0)) &&
            (ambient = XMLParser_get_child_node_by_name(graphic,
              "ambient",0)) &&
            (emission = XMLParser_get_child_node_by_name(graphic,
              "emission",0)) &&
            (specular = XMLParser_get_child_node_by_name(graphic,
              "specular",0)))
          {
            dif[0] = XMLParser_get_attribute_value(diffuse,"red");
            dif[1] = XMLParser_get_attribute_value(diffuse,"green");
            dif[2] = XMLParser_get_attribute_value(diffuse,"blue");
            amb[0] = XMLParser_get_attribute_value(ambient,"red");
            amb[1] = XMLParser_get_attribute_value(ambient,"green");
            amb[2] = XMLParser_get_attribute_value(ambient,"blue");
            emi[0] = XMLParser_get_attribute_value(emission,"red");
            emi[1] = XMLParser_get_attribute_value(emission,"green");
            emi[2] = XMLParser_get_attribute_value(emission,"blue");
            spe[0] = XMLParser_get_attribute_value(specular,"red");
            spe[1] = XMLParser_get_attribute_value(specular,"green");
            spe[2] = XMLParser_get_attribute_value(specular,"blue");
            alpha = XMLParser_get_attribute_value(graphic,"alpha");
            shininess = XMLParser_get_attribute_value(graphic,"shininess");
            Cell_graphic_create_graphical_material(cell_graphic,cmgui_interface,
              dif[0],dif[1],dif[2],
              amb[0],amb[1],amb[2],
              emi[0],emi[1],emi[2],
              spe[0],spe[1],spe[2],
              alpha,shininess);
            for (i=0;i<3;i++)
            {
              if (dif[i]) free(dif[i]);
              if (amb[i]) free(amb[i]);
              if (emi[i]) free(emi[i]);
              if (spe[i]) free(spe[i]);
            }
            if (alpha) free(alpha);
            if (shininess) free(shininess);
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
        free(name);
      }
      else
      {
        display_message(WARNING_MESSAGE,"build_graphics.  "
          "Unable to get the graphic's name");
      }
      position++;
    } /* while (return_code && graphic) */
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
  struct LIST(Cell_graphic) *graphic_list,char *base_path)
/*******************************************************************************
LAST MODIFIED : 20 April 2001

DESCRIPTION :
Uses the information contained in the <parent> DOM element to set-up the
graphical representation of CellML components.
==============================================================================*/
{
  int return_code = 0,dom_position,i;
  char *name;
  void *display,*element,*position,*direction,*scale;
  struct Cell_component *component;
  struct Cell_graphic *graphic;
  char *pos[3],*dir[3],*sca[3];

  ENTER(build_graphical_display);
  if (parent && component_list)
  {
    /* Loop through all the display elements */
    dom_position = 0;
    return_code = 1;
    while (return_code && (display = XMLParser_get_child_node_by_name(parent,
      "display",dom_position)))
    {
      /* Get the component referenced */
      if (element = XMLParser_get_child_node_by_name(display,"component_ref",0))
      {
        if (name = XMLParser_get_attribute_value(element,"name_ref"))
        {
          component = FIND_BY_IDENTIFIER_IN_LIST(Cell_component,name)(
            name,component_list);
          free(name);
          if ((position = XMLParser_get_child_node_by_name(display,
            "position",0)) &&
            (direction = XMLParser_get_child_node_by_name(display,
              "direction",0)) &&
            (scale = XMLParser_get_child_node_by_name(display,
              "scale",0)))
          {
            pos[0] = XMLParser_get_attribute_value(position,"x");
            pos[1] = XMLParser_get_attribute_value(position,"y");
            pos[2] = XMLParser_get_attribute_value(position,"z");
            dir[0] = XMLParser_get_attribute_value(direction,"x");
            dir[1] = XMLParser_get_attribute_value(direction,"y");
            dir[2] = XMLParser_get_attribute_value(direction,"z");
            sca[0] = XMLParser_get_attribute_value(scale,"x");
            sca[1] = XMLParser_get_attribute_value(scale,"y");
            sca[2] = XMLParser_get_attribute_value(scale,"z");
            /* Set-up the graphical transformation matrix */
            if (Cell_component_set_graphical_transformation(component,
              pos[0],pos[1],pos[2],
              dir[0],dir[1],dir[2],
              sca[0],sca[1],sca[2]))
            {
              /* Now need to associate the component with its graphic */
              if (element = XMLParser_get_child_node_by_name(display,
                "graphic_ref",0))
              {
                if (name = XMLParser_get_attribute_value(element,"name_ref"))
                {
                  graphic = FIND_BY_IDENTIFIER_IN_LIST(Cell_graphic,name)(
                    name,graphic_list);
                  Cell_component_set_graphic(component,graphic);
                  free(name);
                }
              }
              else
              {
                /* No graphic reference found, look for a straight graphic
                   definition */
                if (element = XMLParser_get_child_node_by_name(display,
                  "graphic",0))
                {
                  /* build the graphic */
                  build_graphics(display,cmgui_interface,graphic_list,
                    base_path);
                  if (name = XMLParser_get_attribute_value(element,"name"))
                  {
                    graphic = FIND_BY_IDENTIFIER_IN_LIST(Cell_graphic,name)(
                      name,graphic_list);
                    Cell_component_set_graphic(component,graphic);
                    free(name);
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
            for (i=0;i<3;i++)
            {
              if (pos[i]) free(pos[i]);
              if (dir[i]) free(dir[i]);
              if (sca[i]) free(sca[i]);
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
    } /* while (return_code && display) */
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
LAST MODIFIED : 20 April 2001

DESCRIPTION :
Main routine to build the Cell data objects from the DOM tree with the given
<root> element.
==============================================================================*/
{
  void *child;
  struct Cell_component *root_component;

  ENTER(build_from_dom);
  if (root && component_list && variable_list)
  {
    /* Create the units abbreviation look-up table if one is given
     */
    if (child = XMLParser_get_child_node_by_name(root,
      "units_abbreviation_table",0))
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
LAST MODIFIED : 18 April 2001

DESCRIPTION :
Reads in the cell model specified in <filename>. <XMLParser_initialised> is a
pointer to the value in the Cell interface object, used to determine if the
XML parser has been initialised or not.

Returns the display name of the model.
==============================================================================*/
{
  void *cellml_element,*root_element,*element;
  char *display_name = (char *)NULL;
  char *base_path = (char *)NULL;
  char *display_name_xml;
  
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
      /* Destroy any existing document */
      if (cell_input->crim_document)
      {
        CRIM_destroy_document(cell_input->crim_document);
        cell_input->crim_document = (void *)NULL;
      }
      /* Do a CRIM parse of the model file */
      if (cell_input->crim_document = CRIMParse(filename,
        cell_input->copy_tags,cell_input->ref_tags,cell_input->do_validation,
        cell_input->do_namespaces,cell_input->do_expand))
      {
        /* ??? A NULL base path is OK if the document gets through the parse ???
         */
        base_path = CRIM_get_base_path(cell_input->crim_document);
        /* Get the root element of the CellML information - could be either
         * the root element of the CellML DOM document or simply the top
         * element of the CellML description contained in the CELL input file
         */
        if (cellml_element = CRIM_get_element(cell_input->crim_document,
          "cell_model_component",/*refAllowed*/1))
        {
          /* Build the cell data objects */
          build_from_dom(cellml_element,component_list,variable_list);
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
              build_calculation_information(element,cell_calculate,base_path);
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
              build_graphics(element,cmgui_interface,graphic_list,base_path);
              /* And then set-up all the component graphical representation */
              build_graphical_display(element,cmgui_interface,
                component_list,graphic_list,base_path);
            }
            /* grab the display name of the model/experiment - if no display
             * name found, try just the name - need to return a copy of the
             * value returned from the XML library to avoid DEALLOCATE'ing some
             * malloc'ed memory */
            if (!(display_name_xml = XMLParser_get_attribute_value(root_element,
              "display_name")))
            {
              display_name_xml = XMLParser_get_attribute_value(
                root_element,"name");
            }
            if (display_name_xml)
            {
              if (ALLOCATE(display_name,char,strlen(display_name_xml)+1))
              {
                strcpy(display_name,display_name_xml);
              }
              else
              {
                display_name = (char *)NULL;
              }
              free(display_name_xml);
            }
            else
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
        if (base_path)
        {
          DEALLOCATE(base_path);
        }
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

