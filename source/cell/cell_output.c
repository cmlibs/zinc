/*******************************************************************************
FILE : cell_output.c

LAST MODIFIED : 16 June 2001

DESCRIPTION :
Output routines for the cell interface.
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The XML headers */
#include "local_utils/crim.h"
#include "local_utils/xerces_wrapper.h"

#include "cell/cell_input.h"
#include "cell/cell_output.h"
#include "finite_element/import_finite_element.h"

/*
Module objects
--------------
*/
struct Cell_output_variable_iterator
/*******************************************************************************
LAST MODIFIED : 14 November 2000

DESCRIPTION :
A data object used when iterating through a varaible list updating the variable
modification elements.
==============================================================================*/
{
  /* The DOM document object */
  void *dom_document;
  /* The <variable_modifications> parent element */
  void *parent;
}; /* struct Cell_output_variable_iterator */

struct Element_iterator_data
/*******************************************************************************
LAST MODIFIED : 02 February 2001

DESCRIPTION :
A data object used when iterating through the elements in a element group.
==============================================================================*/
{
  struct Cell_cmgui_interface *cmgui_interface;
  FILE *file;
  struct FE_field *grid_field;
  struct FE_field *field;
  int number;
}; /* struct Element_iterator_data */

struct Cell_output
/*******************************************************************************
LAST MODIFIED : 13 November 2000

DESCRIPTION :
A data object used to output cell models into a Cell configuration file.
==============================================================================*/
{
  /* Need a pointer to the cell input object so can get a handle on the current
     model file (as parsed) */
  struct Cell_input *cell_input;
  /* The cell calculation object */
  struct Cell_calculate *cell_calculate;
}; /* struct Cell_output */

/*
Module functions
----------------
*/
#if defined (CELL_DISTRIBUTED)
static int write_FE_element_spatially_varying(struct FE_element *element,
	void *data_void)
/*******************************************************************************
LAST MODIFIED : 02 February 2001

DESCRIPTION :
Iterator function for writing out the spatially varying parameters at each
element point in all the elements
==============================================================================*/
{
	int return_code = 0;
	struct Element_iterator_data *data;
  int *grid_point_numbers,i,number_of_grid_values;
  FE_value *fe_values;
  int *int_values;

	ENTER(write_FE_element_spatially_varying);
	if (element && (data = (struct Element_iterator_data *)data_void))
	{
    /* make sure that the field is
     * the right type of field. Also check that the element is a top level
     * element (so we don't try and write out stuf for all the lines,
     * faces, etc...) */
    if ((CM_ELEMENT == element->cm.type) &&
      FE_element_field_is_grid_based(element,data->field))
    {
      if ((FE_VALUE_VALUE == get_FE_field_value_type(data->field)))
      {
        /* real fields */
        
        /* check the grid point number field */
        if ((INT_VALUE == get_FE_field_value_type(data->grid_field)) &&
          FE_element_field_is_grid_based(element,data->grid_field))
        {
          /* ????????????????????????????????
           * Is this the best way to do this?? Maybe should be getting the
           * values as strings to avoid the int/real difference, but you need
           * to do that at Xi locations, so not suitable for looping through
           * all element points ??
           * ????????????????????????????????
           */
        
          /* get the field values and grid point numbers for all the
           * element points in the element */
          if (get_FE_element_field_component_grid_FE_value_values(element,
            data->field,/*component_number*/0,&fe_values) &&
            get_FE_element_field_component_grid_int_values(element,
              data->grid_field,/*component_number*/0,
              &grid_point_numbers))
          {
            number_of_grid_values =
              get_FE_element_field_number_of_grid_values(element,data->field);
            for (i=0;i<number_of_grid_values;i++)
            {
              fprintf(data->file," Enter collocation point "
                "#s/name [EXIT]: ");
              fprintf(data->file,"%d\n",grid_point_numbers[i]);
              fprintf(data->file," The value for parameter %d is "
                "[ 0.00000D+00]: ",data->number);
              fprintf(data->file,"%g\n",(float)(fe_values[i]));
            }
            return_code = 1;
          }
          else
          {
            display_message(ERROR_MESSAGE,"write_FE_element_spatially_varying. "
              "Unable to get the cell type values or the grid point numbers");
            return_code = 0;
          }
        }
        else
        {
          /*display_message(INFORMATION_MESSAGE,
            "write_FE_element_point_variants. "
            "The cell_type field is not grid based in this element\n");*/
          return_code = 1;
        }
      }
      else if ((INT_VALUE == get_FE_field_value_type(data->field)))
      {
        /* integer fields */
        
        /* check the grid point number field */
        if ((INT_VALUE == get_FE_field_value_type(data->grid_field)) &&
          FE_element_field_is_grid_based(element,data->grid_field))
        {
          /* ????????????????????????????????
           * Is this the best way to do this?? Maybe should be getting the
           * values as strings to avoid the int/real difference, but you need
           * to do that at Xi locations, so not suitable for looping through
           * all element points ??
           * ????????????????????????????????
           */
        
          /* get the variant numbers and grid point numbers for all the
           * element points in the element */
          if (get_FE_element_field_component_grid_int_values(element,
            data->field,/*component_number*/0,&int_values) &&
            get_FE_element_field_component_grid_int_values(element,
              data->grid_field,/*component_number*/0,
              &grid_point_numbers))
          {
            number_of_grid_values=
              get_FE_element_field_number_of_grid_values(element,
                data->field);
            for (i=0;i<number_of_grid_values;i++)
            {
              fprintf(data->file," Enter collocation point "
                "#s/name [EXIT]: %d\n",grid_point_numbers[i]);
              fprintf(data->file," The value for parameter %d is "
                "[ 0.00000D+00]: ",data->number);
              fprintf(data->file,"%d\n",int_values[i]);
            }
            return_code = 1;
          }
          else
          {
            display_message(ERROR_MESSAGE,"write_FE_element_spatially_varying. "
              "Unable to get the cell type values or the grid point numbers");
            return_code = 0;
          }
        }
        else
        {
          /*display_message(INFORMATION_MESSAGE,
            "write_FE_element_point_variants. "
            "The cell_type field is not grid based in this element\n");*/
          return_code = 1;
        }
      }
		}
		else
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"write_FE_element_spatially_varying. "
			"Invalid arguments");
		return_code = 0;
	}
	LEAVE;
	return(return_code);
} /* END write_FE_element_spatially_varying() */
#endif /* defined (CELL_DISTRIBUTED) */

#if defined (CELL_DISTRIBUTED)
static void write_spatial_variable_using_fields(
	int number,char *name,struct Cell_cmgui_interface *cmgui_interface,
  FILE *file,struct GROUP(FE_element) *element_group,
  struct FE_field *grid_field)
/*******************************************************************************
LAST MODIFIED : 02 February 2001

DESCRIPTION :
If the given variable is spatially varying, writes out the information required
in the ipmatc <file>.
==============================================================================*/
{
	struct FE_field *field;
	struct Element_iterator_data element_iterator_data;

	ENTER(write_spatial_variable_using_fields);
	if (name && cmgui_interface && grid_field && element_group)
	{
    /* if the parameter is spatially varying, write out its value
			 at each node */
    if (field = FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)(name,
      Cell_cmgui_interface_get_fe_field_manager(cmgui_interface)))
    {
      if (get_FE_field_FE_field_type(field) == GENERAL_FE_FIELD)
      {
        fprintf(file,"\n");
        fprintf(file," Parameter number %d is [2]:\n",number);
        fprintf(file,"   (1) Piecewise constant (defined by elements)\n");
        fprintf(file,"   (2) Piecewise linear (defined by nodes)\n");
        fprintf(file,"   (3) Defined by grid points\n");
        fprintf(file,"    3\n");
        /* write out the element point values */
        element_iterator_data.file = file;
        element_iterator_data.cmgui_interface = cmgui_interface;
        element_iterator_data.number = number;
        element_iterator_data.field = field;
        element_iterator_data.grid_field = grid_field;
        FOR_EACH_OBJECT_IN_GROUP(FE_element)(
          write_FE_element_spatially_varying,(void *)(&element_iterator_data),
          element_group);
        /* end the parameter data */
        fprintf(file," Enter collocation point #s/name [EXIT]: 0\n");
      }
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_spatial_variable_using_fields. "
			"Invalid arguments");
	}
	LEAVE;
} /* write_spatial_variable_using_fields() */
#endif /* defined (CELL_DISTRIBUTED) */

#if defined (CELL_DISTRIBUTED)
static int write_FE_element_point_variants(struct FE_element *element,
  void *data_void)
/*******************************************************************************
LAST MODIFIED : 02 February 2001

DESCRIPTION :
Iterator function for writing out the variant of each element point in the
<element>. Only works on top level elements.
==============================================================================*/
{
	int return_code = 0;
	struct Element_iterator_data *data;
	struct FE_field *field;
	int *variants,*grid_point_numbers,i,number_of_grid_values;

	ENTER(write_FE_element_point_variants);
	if (element && (data = (struct Element_iterator_data *)data_void))
	{
		/* get the cell_type (variant number) field and make sure that it is
     * the right type of field. Also check that the element is a top level
     * element (so we don't try and write out stuf for all the lines,
     * faces, etc...) */
		if ((CM_ELEMENT == element->cm.type) &&
      (field = FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)("cell_type",
        Cell_cmgui_interface_get_fe_field_manager(data->cmgui_interface))) &&
      (INT_VALUE == get_FE_field_value_type(field)) &&
      FE_element_field_is_grid_based(element,field))
    {
      /* check the grid point number field */
      if ((INT_VALUE == get_FE_field_value_type(data->grid_field)) &&
        FE_element_field_is_grid_based(element,data->grid_field))
      {
        /* get the variant numbers and grid point numbers for all the
         * element points in the element */ 
        if (get_FE_element_field_component_grid_int_values(element,
          field,/*component_number*/0,&variants) &&
          get_FE_element_field_component_grid_int_values(element,
            data->grid_field,/*component_number*/0,&grid_point_numbers))
        {
          number_of_grid_values=
            get_FE_element_field_number_of_grid_values(element,field);
          for (i=0;i<number_of_grid_values;i++)
          {
            fprintf(data->file," Enter collocation point "
              "#s/name [EXIT]: ");
            fprintf(data->file,"%d\n",grid_point_numbers[i]);
            fprintf(data->file," The cell variant number is "
              "[1]: %d\n",variants[i]);            
          }
          return_code = 1;
        }
        else
        {
          display_message(ERROR_MESSAGE,"write_FE_element_point_variants. "
            "Unable to get the cell type values or the grid point numbers");
          return_code = 0;
        }
      }
      else
      {
        /*display_message(INFORMATION_MESSAGE,
          "write_FE_element_point_variants. "
          "The cell_type field is not grid based in this element\n");*/
        return_code = 1;
      }
		}
		else
		{
			/*display_message(INFORMATION_MESSAGE,"write_FE_element_point_variants. "
				"No cell_type field in this element\n");*/
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"write_FE_element_point_variants. "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	return(return_code);
} /* write_FE_element_point_variants() */
#endif /* defined (CELL_DISTRIBUTED) */

#if defined (CELL_DISTRIBUTED)
static int check_field_type(struct FE_field *field,void *field_type_void)
/*******************************************************************************
LAST MODIFIED : 02 February 2001

DESCRIPTION :
Returns true if the FIELD_TYPE specified in the <field_type_void> matches
the FIELD_TYPE of the <field>.
==============================================================================*/
{
	int return_code = 0;
	enum FE_field_type field_type;

	ENTER(check_field_type);
	if (field && (field_type = (enum FE_field_type)field_type_void))
	{
		if (field_type == get_FE_field_FE_field_type(field))
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"check_field_type. "
			"Invalid arguments");
		return_code = 0;
	}
	LEAVE;
	return(return_code);
} /* check_field_type() */
#endif /* defined (CELL_DISTRIBUTED) */

#if defined (CELL_DISTRIBUTED)
static void write_real_variable_using_fields(int number,float value,char *name,
  struct Cell_cmgui_interface *cmgui_interface,int variant,FILE *file,
  int spatial_check)
/*******************************************************************************
LAST MODIFIED : 02 February 2001

DESCRIPTION :
Writes out the variable's line in a IPCELL file. If <spatial_check> is 0, then
no check is perfromed to see if the variable is spatially varying or not -
assumed not!
==============================================================================*/
{
	struct FE_field *field;
	FE_value fe_value;
	float parameter_value;
  char spatial;

	ENTER(write_real_variable_using_fields);
	if (name && cmgui_interface && file)
	{
    if ((field = FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)(name,
      Cell_cmgui_interface_get_fe_field_manager(cmgui_interface))) &&
      (FE_VALUE_VALUE == get_FE_field_value_type(field)))
    {
      if (spatial_check)
      {
        if (get_FE_field_FE_field_type(field) == INDEXED_FE_FIELD)
        {
          spatial = ' ';
          if (get_FE_field_FE_value_value(field,variant,&fe_value))
          {
            parameter_value = (float)fe_value;
          }
          else
          {
            display_message(ERROR_MESSAGE,
              "write_real_variable_using_fields. "
              "Unable to get the indexed field value");
            parameter_value = -999999.99;
          }
        }
        else
        {
          spatial = '*';
          parameter_value = value;
        }
      }
      else
      {
        spatial = ' ';
        parameter_value = value;
      }
    }
    else
    {
      spatial = ' ';
      parameter_value = value;
    }
    fprintf(file,"  %3d%c %9.6E \t%s\n",number,spatial,parameter_value,name);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_real_variable_using_fields. "
			"Invalid arguments");
	}
	LEAVE;
} /* write_real_variable_using_fields() */
#endif /* defined (CELL_DISTRIBUTED) */

#if defined (CELL_DISTRIBUTED)
static void write_integer_variable_using_fields(int number,int value,
  char *name,struct Cell_cmgui_interface *cmgui_interface,int variant,
  FILE *file)
/*******************************************************************************
LAST MODIFIED : 02 February 2001

DESCRIPTION :
Writes out the variable's line in a IPCELL file.
==============================================================================*/
{
	struct FE_field *field;
	int int_value;
	int parameter_value;
  char spatial;

	ENTER(write_integer_variable_using_fields);
	if (name && cmgui_interface && file)
	{
    if ((field = FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)(name,
      Cell_cmgui_interface_get_fe_field_manager(cmgui_interface))) &&
      (INT_VALUE == get_FE_field_value_type(field)))
    {
      if (get_FE_field_FE_field_type(field) == INDEXED_FE_FIELD)
      {
        spatial = ' ';
        if (get_FE_field_int_value(field,variant,&int_value))
        {
          parameter_value = (int)int_value;
        }
        else
        {
          display_message(ERROR_MESSAGE,
            "write_integer_variable_using_fields. "
            "Unable to get the indexed field value");
          parameter_value = -999999;
        }
      }
      else
      {
        spatial = '*';
        parameter_value = value;
      }
    }
    else
    {
      spatial = ' ';
      parameter_value = value;
    }
    fprintf(file,"  %3d%c %d \t%s\n",number,spatial,parameter_value,name);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_integer_variable_using_fields. "
			"Invalid arguments");
	}
	LEAVE;
} /* write_integer_variable_using_fields() */
#endif /* defined (CELL_DISTRIBUTED) */

static int create_variable_modification_element(struct Cell_variable *variable,
  void *iterator_object_void)
/*******************************************************************************
LAST MODIFIED : 14 November 2000

DESCRIPTION :
Iterator function used to create the required variable modification elements
for the Cell configuration file.
==============================================================================*/
{
  int return_code = 0;
  struct Cell_output_variable_iterator *iterator_object;
  void *element;
  char *string;

  ENTER(create_variable_modification_element);
  if (variable && (iterator_object =
    (struct Cell_output_variable_iterator *)iterator_object_void) &&
    iterator_object->dom_document)
  {
    if (Cell_variable_get_changed(variable))
    {
      /* The variable has changed, so requires a variable modification
         element */
      if (iterator_object->parent == (void *)NULL)
      {
        /* Need to create the parent element */
        if (iterator_object->parent = XMLParser_create_dom_element(
          iterator_object->dom_document,"variable_modifications"))
        {
          /* and append it to the root element of the DOM document */
          XMLParser_append_dom_node(
            XMLParser_get_dom_root_element(iterator_object->dom_document),
            iterator_object->parent);
        }
        else
        {
          display_message(ERROR_MESSAGE,
            "create_variable_modification_element.  "
            "Unable to create the <varaible_modifications> element");
          return_code = 0;
        }
      }
      if (iterator_object->parent)
      {
        /* Have the parent, now add the <modify_variable> element for the
             current variable */          
        if (element = XMLParser_create_dom_element(
          iterator_object->dom_document,"modify_variable"))
        {
          /* and append it to the parent */
          XMLParser_append_dom_node(iterator_object->parent,element);
          /* Now set the attribute values */
          string = Cell_variable_get_name(variable);
          XMLParser_dom_element_set_attribute(element,"name_ref",string);
          if (string) DEALLOCATE(string);
          string = Cell_variable_get_value_as_string(variable);
          XMLParser_dom_element_set_attribute(element,"value",string);
          if (string) DEALLOCATE(string);
          return_code = 1;
        }
        else
        {
          display_message(ERROR_MESSAGE,
            "create_variable_modification_element.  "
            "Unable to create the <varaible_modifications> element");
          return_code = 0;
        }
      }
    }
    else
    {
      /* The variable has not changed, so do nothing */
      return_code = 1;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"create_variable_modification_element.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* create_variable_modification_element() */

static int update_model_calculation_information(
  struct Cell_calculate *calculate,void *element)
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
Updates the attribute values of the <element> to be consistent with the
<calculate> object.
==============================================================================*/
{
  int return_code = 0;
  char *string;

  ENTER(update_model_calculation_information);
  if (calculate && element)
  {
    if (string = Cell_calculate_get_model_routine_name(calculate))
    {
      XMLParser_dom_element_set_attribute(element,"model_routine_name",string);
      DEALLOCATE(string);
    }
    if (string = Cell_calculate_get_dso_name(calculate))
    {
      XMLParser_dom_element_set_attribute(element,"model_dso_name",string);
      DEALLOCATE(string);
    }
    if (string = Cell_calculate_get_intg_routine_name(calculate))
    {
      XMLParser_dom_element_set_attribute(element,"integrator_routine_name",
        string);
      DEALLOCATE(string);
    }
    if (string = Cell_calculate_get_intg_dso_name(calculate))
    {
      XMLParser_dom_element_set_attribute(element,"integrator_dso_name",string);
      DEALLOCATE(string);
    }
    if (string = Cell_calculate_get_start_time_as_string(calculate))
    {
      XMLParser_dom_element_set_attribute(element,"tstart",string);
      DEALLOCATE(string);
    }
    if (string = Cell_calculate_get_end_time_as_string(calculate))
    {
      XMLParser_dom_element_set_attribute(element,"tend",string);
      DEALLOCATE(string);
    }
    if (string = Cell_calculate_get_dt_as_string(calculate))
    {
      XMLParser_dom_element_set_attribute(element,"dt",string);
      DEALLOCATE(string);
    }
    if (string = Cell_calculate_get_tabt_as_string(calculate))
    {
      XMLParser_dom_element_set_attribute(element,"tabt",string);
      DEALLOCATE(string);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"update_model_calculation_information.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* update_model_calculation_information() */

static int update_cmiss_mapping_information(void *cmiss_mapping,
  struct LIST(Cell_variable) *variable_list)
/*******************************************************************************
LAST MODIFIED : 14 April 2001

DESCRIPTION :
Checks the information contained in the <cmiss_mapping> DOM element is
up-to-date with the current set of variables in the <variable_list>, updating
the elements when required. Currently the only information in the CMISS mapping
the Cell controls is the UnEmap plotting.
==============================================================================*/
{
  int return_code = 0,position;
  char *name;
  void *map_variable;
  struct Cell_variable *variable;

  ENTER(update_cmiss_mapping_information);
  if (cmiss_mapping && variable_list)
  {
    /* Loop through all the "map_variable" children of the cmiss_mapping
     * element and make sure that he information is up-to-date
     */
    position = 0;
    return_code = 1;
    /* Get all the <map_variable> children of the parent node */
    while (return_code && (map_variable = XMLParser_get_child_node_by_name(
      cmiss_mapping,"map_variable",position)))
    {
      /* Get the variable name */
      if (name = XMLParser_get_attribute_value(map_variable,"name_ref"))
      {
        /* Get the corresponding Cell variable */
        if (variable = FIND_BY_IDENTIFIER_IN_LIST(Cell_variable,name)(name,
          variable_list))
        {
          if (Cell_variable_variable_has_unemap_interface(variable))
          {
            XMLParser_dom_element_set_attribute(map_variable,"plot","unemap");
          }
          else
          {
            XMLParser_dom_element_set_attribute(map_variable,"plot","none");
          }
        }
        else
        {
          /* ?? ignore ?? */
        }
        free(name);
      }
      else
      {
        /* ?? ignore ?? */
      }
      position++;
    } /* while (return_code && map_variable) */
  }
  else
  {
    display_message(ERROR_MESSAGE,"update_cmiss_mapping_information.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* update_cmiss_mapping_information() */

/*
Global functions
----------------
*/
struct Cell_output *CREATE(Cell_output)(struct Cell_input *cell_input,
  struct Cell_calculate *cell_calculate)
/*******************************************************************************
LAST MODIFIED : 13 November 2000

DESCRIPTION :
Creates a Cell_output object.
==============================================================================*/
{
  struct Cell_output *cell_output = (struct Cell_output *)NULL;

  ENTER(CREATE(Cell_output));
  if (cell_input && cell_calculate)
  {
    if (ALLOCATE(cell_output,struct Cell_output,1))
    {
      /* Initialise the data fields */
      cell_output->cell_input = cell_input;
      cell_output->cell_calculate = cell_calculate;
    }
    else
    {
      display_message(ERROR_MESSAGE,"CREATE(Cell_output).  "
        "Unable to allocate memory for the cell output object");
      cell_output = (struct Cell_output *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Cell_output).  "
      "Invalid argument(s)");
    cell_output = (struct Cell_output *)NULL;
  }
  LEAVE;
  return(cell_output);
} /* CREATE(Cell_output)() */

int DESTROY(Cell_output)(struct Cell_output **cell_output_address)
/*******************************************************************************
LAST MODIFIED : 13 November 2000

DESCRIPTION :
Destroys a Cell_output object.
==============================================================================*/
{
  int return_code = 0;

  ENTER(DESTROY(Cell_output));
  if (cell_output_address)
  {
    DEALLOCATE(*cell_output_address);
    *cell_output_address = (struct Cell_output *)NULL;
    return_code = 1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"DESTROY(Cell_output).  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* DESTROY(Cell_output)() */

int Cell_output_write_model_to_file(struct Cell_output *cell_output,
  char *filename,struct LIST(Cell_variable) *variable_list)
/*******************************************************************************
LAST MODIFIED : 13 November 2000

DESCRIPTION :
Writes the current Cell configuration to a configuration file.
==============================================================================*/
{
  int return_code = 0;
  void *crim_document;
  void *config_dom_document;
  void *element;
  struct Cell_output_variable_iterator iterator_object;

  ENTER(Cell_output_write_model_to_file);
  /* Check arguments */
  if (cell_output && filename)
  {
    /* Get the current CRIM document */
    if (crim_document = Cell_input_get_crim_document(cell_output->cell_input))
    {
      /* Get the current DOM document for the configuration file - this should
         always be the top level entry of the CRIM document ??? */
      if (config_dom_document = CRIM_get_named_document(crim_document,
        "topLevel"))
      {
        /* Update the current configuration DOM document and then write it out
         */
        /* Start with the calculation information */
        if (element = CRIM_get_element(crim_document,"model_calculation",
          /*refAllowed*/0))
        {
          /* Check and update the calculation information */
          update_model_calculation_information(cell_output->cell_calculate,
            element);
        }
        else
        {
          /* Check if a model_calculation element needs to be added, and do
             so */
        }
        /* Then update the CMISS mapping information - the only thing that
           can change with this is the UnEmap plotting */
        if (element = CRIM_get_element(crim_document,"cmiss_mapping",
          /*refAllowed*/0))
        {
          update_cmiss_mapping_information(element,variable_list);
        }
        /* Now do the variable modifications, first remove any existing
           modifications */
        if (element = CRIM_get_element(crim_document,
          "variable_modifications",/*refAllowed*/0))
        {
          XMLParser_remove_dom_node_child(element);
        }
        /* and then loop through all the variables and add any required
           variable modifications */
        iterator_object.dom_document = config_dom_document;
        iterator_object.parent = (void *)NULL;
        FOR_EACH_OBJECT_IN_LIST(Cell_variable)(
          create_variable_modification_element,(void *)(&iterator_object),
          variable_list);
        /* Finally, write out the config DOM document */
        XMLParser_write_dom_document(config_dom_document,filename,
          /*do_escapes*/1,/*add_returns*/0);
        return_code = 1;
      }
      else
      {
        display_message(ERROR_MESSAGE,"Cell_output_write_model_to_file.  "
          "Unable to get the current configuration DOM document");
        return_code = 0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_output_write_model_to_file.  "
        "Unable to get the current CRIM document");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_output_write_model_to_file.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_output_write_model_to_file() */

int Cell_output_write_model_to_ipcell_file(char *filename,
  struct LIST(Cell_variable) *variable_list)
/*******************************************************************************
LAST MODIFIED : 11 January 2001

DESCRIPTION :
Writes the current Cell model to a ipcell file
==============================================================================*/
{
  int return_code = 0,i;
  struct Cell_cmiss_interface_arrays *cmiss_interface_arrays;
  FILE *file;

  ENTER(Cell_output_write_model_to_ipcell_file);
  /* Check arguments */
  if (filename && variable_list)
  {
    /* Build the CMISS arrays */
    if (cmiss_interface_arrays =
      CREATE(Cell_cmiss_interface_arrays)(variable_list))
    {
      /* Open the file */
      if (file = fopen(filename,"w"))
      {
        /* Write out the file heading */
        fprintf(file," CMISS Version 1.21 ipcell File Version 1\n");
        fprintf(file," Heading: ipcell file generated by CELL\n\n");
        fprintf(file," The number of cell model variants is: 1\n");
        fprintf(file," The number of state variables is: %d\n",
          cmiss_interface_arrays->size_y);
        fprintf(file," The number of ODE variables is: %d\n",
          cmiss_interface_arrays->size_y);
        fprintf(file," The number of derived variables is: %d\n",
          cmiss_interface_arrays->size_derived);
        fprintf(file," The number of cell model parameters is: %d\n",
          cmiss_interface_arrays->size_model);
        fprintf(file," The number of cell control parameters is: %d\n",
          cmiss_interface_arrays->size_control);
        fprintf(file," The number of cell material parameters is: %d\n",
          cmiss_interface_arrays->size_parameters);
        fprintf(file," The number of cell protocol parameters is: %d\n",
          cmiss_interface_arrays->size_protocol);
        fprintf(file,
          " The number of additional integer input parameters is: %d\n",
          cmiss_interface_arrays->size_aii);
        fprintf(file,
          " The number of additional integer output parameters is: %d\n",
          cmiss_interface_arrays->size_aio);
        fprintf(file," The number of additional real input parameters is: %d\n",
          cmiss_interface_arrays->size_ari);
        fprintf(file,
          " The number of additional real output parameters is: %d\n",
          cmiss_interface_arrays->size_aro);
        if (cmiss_interface_arrays->size_y > 0)
        {
          fprintf(file,"\n");
          fprintf(file," State variables:\n");
          fprintf(file," Cell variant 1:\n");
          for (i=0;i<cmiss_interface_arrays->size_y;i++)
          {
            fprintf(file,"  %3d %9.6E \t%s\n",i+1,
              (float)(cmiss_interface_arrays->y[i]),
              cmiss_interface_arrays->y_names[i]);
          }
        }
        if (cmiss_interface_arrays->size_model > 0)
        {
          fprintf(file,"\n");
          fprintf(file," Model variables:\n");
          fprintf(file," Cell variant 1:\n");
          for (i=0;i<cmiss_interface_arrays->size_model;i++)
          {
            fprintf(file,"  %3d %d \t%s\n",i+1,
              (int)(cmiss_interface_arrays->model[i]),
              cmiss_interface_arrays->model_names[i]);
          }
        }
        if (cmiss_interface_arrays->size_control > 0)
        {
          fprintf(file,"\n");
          fprintf(file," Control variables:\n");
          fprintf(file," Cell variant 1:\n");
          for (i=0;i<cmiss_interface_arrays->size_control;i++)
          {
            fprintf(file,"  %3d %d \t%s\n",i+1,
              (int)(cmiss_interface_arrays->control[i]),
              cmiss_interface_arrays->control_names[i]);
          }
        }
        if (cmiss_interface_arrays->size_parameters > 0)
        {
          fprintf(file,"\n");
          fprintf(file," Parameter variables:\n");
          fprintf(file," Cell variant 1:\n");
          for (i=0;i<cmiss_interface_arrays->size_parameters;i++)
          {
            fprintf(file,"  %3d %9.6E \t%s\n",i+1,
              (float)(cmiss_interface_arrays->parameters[i]),
              cmiss_interface_arrays->parameters_names[i]);
          }
        }
        if (cmiss_interface_arrays->size_protocol > 0)
        {
          fprintf(file,"\n");
          fprintf(file," Protocol variables:\n");
          fprintf(file," Cell variant 1:\n");
          for (i=0;i<cmiss_interface_arrays->size_protocol;i++)
          {
            fprintf(file,"  %3d %9.6E \t%s\n",i+1,
              (float)(cmiss_interface_arrays->protocol[i]),
              cmiss_interface_arrays->protocol_names[i]);
          }
        }
        if (cmiss_interface_arrays->size_aii > 0)
        {
          fprintf(file,"\n");
          fprintf(file," Additional integer input variables:\n");
          fprintf(file," Cell variant 1:\n");
          for (i=0;i<cmiss_interface_arrays->size_aii;i++)
          {
            fprintf(file,"  %3d %d \t%s\n",i+1,
              (int)(cmiss_interface_arrays->aii[i]),
              cmiss_interface_arrays->aii_names[i]);
          }
        }
        if (cmiss_interface_arrays->size_ari > 0)
        {
          fprintf(file,"\n");
          fprintf(file," Additional real input variables:\n");
          fprintf(file," Cell variant 1:\n");
          for (i=0;i<cmiss_interface_arrays->size_ari;i++)
          {
            fprintf(file,"  %3d %9.6E \t%s\n",i+1,
              (float)(cmiss_interface_arrays->ari[i]),
              cmiss_interface_arrays->ari_names[i]);
          }
        }
        fclose(file);
        return_code = 1;
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "Cell_output_write_model_to_ipcell_file.  "
          "Unable to open \"%s\" for writing",filename);
        return_code = 0;
      }
      DESTROY(Cell_cmiss_interface_arrays)(&cmiss_interface_arrays);
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_output_write_model_to_ipcell_file.  "
        "Unable to create the CMISS arrays");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_output_write_model_to_ipcell_file.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_output_write_model_to_ipcell_file() */

#if defined (CELL_DISTRIBUTED)
int Cell_output_write_model_to_ipcell_file_from_fields(char *filename,
  struct LIST(Cell_variable) *variable_list,
  struct Cell_cmgui_interface *cmgui_interface)
/*******************************************************************************
LAST MODIFIED : 02 February 2001

DESCRIPTION :
Writes the current Cell model to a ipcell file, using any defined fields
==============================================================================*/
{
  int return_code = 0,i,number_of_variants,variant;
  struct Cell_cmiss_interface_arrays *cmiss_interface_arrays;
  FILE *file;
  struct FE_field *field;

  ENTER(Cell_output_write_model_to_ipcell_file_from_fields);
  /* Check arguments */
  if (filename && variable_list && cmgui_interface)
  {
    /* find an indexed field, and get its number of values - this will
       always give the number of variants ??? */
    if (field = FIRST_OBJECT_IN_MANAGER_THAT(FE_field)(
      check_field_type,(void *)INDEXED_FE_FIELD,
      Cell_cmgui_interface_get_fe_field_manager(cmgui_interface)))
    {
      number_of_variants = get_FE_field_number_of_values(field);
    }
    else
    {
      display_message(WARNING_MESSAGE,
        "Cell_output_write_model_to_ipcell_file_from_fields.  "
        "Unable to find an indexed field, assuming 1 variant");
      number_of_variants = 1;
    }
    /* Build the CMISS arrays */
    if ((number_of_variants > 0) && (cmiss_interface_arrays =
      CREATE(Cell_cmiss_interface_arrays)(variable_list)))
    {
      /* Open the file */
      if (file = fopen(filename,"w"))
      {
        /* Write out the file heading */
        fprintf(file," CMISS Version 1.21 ipcell File Version 1\n");
        fprintf(file," Heading: ipcell file generated by CELL\n\n");
        fprintf(file," The number of cell model variants is: %d\n",
          number_of_variants);
        fprintf(file," The number of state variables is: %d\n",
          cmiss_interface_arrays->size_y);
        fprintf(file," The number of ODE variables is: %d\n",
          cmiss_interface_arrays->size_y);
        fprintf(file," The number of derived variables is: %d\n",
          cmiss_interface_arrays->size_derived);
        fprintf(file," The number of cell model parameters is: %d\n",
          cmiss_interface_arrays->size_model);
        fprintf(file," The number of cell control parameters is: %d\n",
          cmiss_interface_arrays->size_control);
        fprintf(file," The number of cell material parameters is: %d\n",
          cmiss_interface_arrays->size_parameters);
        fprintf(file," The number of cell protocol parameters is: %d\n",
          cmiss_interface_arrays->size_protocol);
        fprintf(file,
          " The number of additional integer input parameters is: %d\n",
          cmiss_interface_arrays->size_aii);
        fprintf(file,
          " The number of additional integer output parameters is: %d\n",
          cmiss_interface_arrays->size_aio);
        fprintf(file," The number of additional real input parameters is: %d\n",
          cmiss_interface_arrays->size_ari);
        fprintf(file,
          " The number of additional real output parameters is: %d\n",
          cmiss_interface_arrays->size_aro);
        if (cmiss_interface_arrays->size_y > 0)
        {
          fprintf(file,"\n");
          fprintf(file," State variables:\n");
          for (variant=0;variant<number_of_variants;variant++)
          {
            fprintf(file," Cell variant %d:\n",variant+1);
            for (i=0;i<cmiss_interface_arrays->size_y;i++)
            {
              write_real_variable_using_fields(i+1,
                (float)(cmiss_interface_arrays->y[i]),
                cmiss_interface_arrays->y_names[i],
                cmgui_interface,variant,file,0);
            }
          }
        }
        if (cmiss_interface_arrays->size_model > 0)
        {
          fprintf(file,"\n");
          fprintf(file," Model variables:\n");
          for (variant=0;variant<number_of_variants;variant++)
          {
            fprintf(file," Cell variant %d:\n",variant+1);
            for (i=0;i<cmiss_interface_arrays->size_model;i++)
            {
              write_integer_variable_using_fields(i+1,
                (int)(cmiss_interface_arrays->model[i]),
                cmiss_interface_arrays->model_names[i],
                cmgui_interface,variant,file);
            }
          }
        }
        if (cmiss_interface_arrays->size_control > 0)
        {
          fprintf(file,"\n");
          fprintf(file," Control variables:\n");
          for (variant=0;variant<number_of_variants;variant++)
          {
            fprintf(file," Cell variant %d:\n",variant+1);
            for (i=0;i<cmiss_interface_arrays->size_control;i++)
            {
              write_integer_variable_using_fields(i+1,
                (int)(cmiss_interface_arrays->control[i]),
                cmiss_interface_arrays->control_names[i],
                cmgui_interface,variant,file);
            }
          }
        }
        if (cmiss_interface_arrays->size_parameters > 0)
        {
          fprintf(file,"\n");
          fprintf(file," Parameter variables:\n");
          for (variant=0;variant<number_of_variants;variant++)
          {
            fprintf(file," Cell variant %d:\n",variant+1);
            for (i=0;i<cmiss_interface_arrays->size_parameters;i++)
            {
              write_real_variable_using_fields(i+1,
                (float)(cmiss_interface_arrays->parameters[i]),
                cmiss_interface_arrays->parameters_names[i],
                cmgui_interface,variant,file,1);
            }
          }
        }
        if (cmiss_interface_arrays->size_protocol > 0)
        {
          fprintf(file,"\n");
          fprintf(file," Protocol variables:\n");
          for (variant=0;variant<number_of_variants;variant++)
          {
            fprintf(file," Cell variant %d:\n",variant+1);
            for (i=0;i<cmiss_interface_arrays->size_protocol;i++)
            {
              write_real_variable_using_fields(i+1,
                (float)(cmiss_interface_arrays->protocol[i]),
                cmiss_interface_arrays->protocol_names[i],
                cmgui_interface,variant,file,1);
            }
          }
        }
        if (cmiss_interface_arrays->size_aii > 0)
        {
          fprintf(file,"\n");
          fprintf(file," Additional integer input variables:\n");
          for (variant=0;variant<number_of_variants;variant++)
          {
            fprintf(file," Cell variant %d:\n",variant+1);
            for (i=0;i<cmiss_interface_arrays->size_aii;i++)
            {
              write_integer_variable_using_fields(i+1,
                (int)(cmiss_interface_arrays->aii[i]),
                cmiss_interface_arrays->aii_names[i],
                cmgui_interface,variant,file);
            }
          }
        }
        if (cmiss_interface_arrays->size_ari > 0)
        {
          fprintf(file,"\n");
          fprintf(file," Additional real input variables:\n");
          for (variant=0;variant<number_of_variants;variant++)
          {
            fprintf(file," Cell variant %d:\n",variant+1);
            for (i=0;i<cmiss_interface_arrays->size_ari;i++)
            {
              write_real_variable_using_fields(i+1,
                (float)(cmiss_interface_arrays->ari[i]),
                cmiss_interface_arrays->ari_names[i],
                cmgui_interface,variant,file,1);
            }
          }
        }
        fclose(file);
        return_code = 1;
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "Cell_output_write_model_to_ipcell_file_from_fields.  "
          "Unable to open \"%s\" for writing",filename);
        return_code = 0;
      }
      DESTROY(Cell_cmiss_interface_arrays)(&cmiss_interface_arrays);
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "Cell_output_write_model_to_ipcell_file_from_fields.  "
        "Unable to create the CMISS arrays");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_output_write_model_to_ipcell_file_from_fields.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_output_write_model_to_ipcell_file_from_fields() */
#endif /* defined (CELL_DISTRIBUTED) */

#if defined (CELL_DISTRIBUTED)
int Cell_output_write_model_to_ipmatc_file_from_fields(char *filename,
  void *element_group_void,void *grid_field_void,
  struct Cell_cmgui_interface *cmgui_interface,
  struct LIST(Cell_variable) *variable_list)
/*******************************************************************************
LAST MODIFIED : 02 February 2001

DESCRIPTION :
Exports the spatially varying variables to a ipmatc file, from element based
fields found in the element group. <grid_field> is used as the grid point
number field.
==============================================================================*/
{
	int return_code = 0,i;
	FILE *file;
  struct GROUP(FE_element) *element_group;
  struct FE_field *grid_field;
  struct Element_iterator_data element_iterator_data;
  struct Cell_cmiss_interface_arrays *cmiss_interface_arrays;

	ENTER(Cell_output_write_model_to_ipmatc_file_from_fields);
	if (filename && cmgui_interface && variable_list &&
    (element_group = (struct GROUP(FE_element) *)element_group_void) &&
    (grid_field = (struct FE_field *)grid_field_void) &&
		(file = fopen(filename,"w")) &&
    (cmiss_interface_arrays =
      CREATE(Cell_cmiss_interface_arrays)(variable_list)))
  {
    /* write out the header, etc.. */
    fprintf(file," CMISS Version 1.21 ipmatc File Version 2\n");
    fprintf(file," Heading: ipmatc file generated by CELL\n");
    fprintf(file,"\n");
    fprintf(file," Enter the cell variant for each collocation point:\n");
    /* write out the variant for each node in the group */
    element_iterator_data.cmgui_interface = cmgui_interface;
    element_iterator_data.file = file;
    element_iterator_data.grid_field = grid_field;
    FOR_EACH_OBJECT_IN_GROUP(FE_element)(write_FE_element_point_variants,
      (void *)(&element_iterator_data),element_group);
    /* end the variant data */
    fprintf(file," Enter collocation point #s/name [EXIT]: 0\n");
    fprintf(file,"\n");
    /* now loop through each of the arrays and write out the spatially
       variables */
    fprintf(file," State variables:\n");
    /*        for (i=0;i<cmiss_interface_arrays->size_y;i++) */
    /*        { */
    /*          write_spatial_variable_using_fields(i+1, */
    /*            cmiss_interface_arrays->y_names[i], */
    /*            cmgui_interface,file,element_group,grid_field); */
    /*        } */
    fprintf(file,"\n");
    fprintf(file," Model variables:\n");
    for (i=0;i<cmiss_interface_arrays->size_model;i++)
    {
      write_spatial_variable_using_fields(i+1,
        cmiss_interface_arrays->model_names[i],
        cmgui_interface,file,element_group,grid_field);
    }
    fprintf(file,"\n");
    fprintf(file," Control variables:\n");
    for (i=0;i<cmiss_interface_arrays->size_control;i++)
    {
      write_spatial_variable_using_fields(i+1,
        cmiss_interface_arrays->control_names[i],
        cmgui_interface,file,element_group,grid_field);
    }
    fprintf(file,"\n");
    fprintf(file," Parameter variables:\n");
    for (i=0;i<cmiss_interface_arrays->size_parameters;i++)
    {
      write_spatial_variable_using_fields(i+1,
        cmiss_interface_arrays->parameters_names[i],
        cmgui_interface,file,element_group,grid_field);
    }
    fprintf(file,"\n");
    fprintf(file," Protocol variables:\n");
    for (i=0;i<cmiss_interface_arrays->size_protocol;i++)
    {
      write_spatial_variable_using_fields(i+1,
        cmiss_interface_arrays->protocol_names[i],
        cmgui_interface,file,element_group,grid_field);
    }
    fprintf(file,"\n");
    fprintf(file," Additional integer input variables:\n");
    for (i=0;i<cmiss_interface_arrays->size_aii;i++)
    {
      write_spatial_variable_using_fields(i+1,
        cmiss_interface_arrays->aii_names[i],
        cmgui_interface,file,element_group,grid_field);
    }
    fprintf(file,"\n");
    fprintf(file," Additional real input variables:\n");
    for (i=0;i<cmiss_interface_arrays->size_ari;i++)
    {
      write_spatial_variable_using_fields(i+1,
        cmiss_interface_arrays->ari_names[i],
        cmgui_interface,file,element_group,grid_field);
    }
    return_code = 1;
    fclose(file);
    DESTROY(Cell_cmiss_interface_arrays)(&cmiss_interface_arrays);
	}
	else
	{
		display_message(ERROR_MESSAGE,
      "Cell_output_write_model_to_ipmatc_file_from_fields.  "
			"Invalid arguments");
		return_code = 0;
	}
	LEAVE;
	return(return_code);
} /* END Cell_output_write_model_to_ipmatc_file_from_fields() */
#endif /* defined (CELL_DISTRIBUTED) */
