/*******************************************************************************
FILE : output.c

LAST MODIFIED : 29 November 1999

DESCRIPTION :
Functions for handling all file output for CELL.
==============================================================================*/
#include <stdio.h>
#include "cell/cell_window.h"
#include "cell/cell_parameter.h"
#include "cell/cell_variable.h"
#include "cell/calculate.h"
#include "cell/cmgui_connection.h"
#include "curve/control_curve.h"

#define CELL_SPATIALLY_VARYING  "* \0"
#define CELL_SPATIALLY_CONSTANT " \0"

/*
Local types
===========
*/
enum Array_value_type
/*******************************************************************************
LAST MODIFIED : 25 May 1999

DESCRIPTION :
Stores the information required to write the ipcell files
==============================================================================*/
{
  ARRAY_VALUE_INTEGER,
  ARRAY_VALUE_REAL
}; /* Array_value_type */

struct Cell_array_information
/*******************************************************************************
LAST MODIFIED : 25 May 1999

DESCRIPTION :
Stores the information required to write the ipcell files
==============================================================================*/
{
  enum Array_value_type type;
  int number;
  char spatial[2];
  char *label;
  union
  {
    int integer_value;
    float real_value;
  } value;
  struct Cell_array_information *next;
}; /* Cell_array_information */

struct Output_data
/*******************************************************************************
LAST MODIFIED : 22 September 1999

DESCRIPTION :
Callback structure for the node group iterator function write_FE_node_variants
==============================================================================*/
{
	FILE *file;
	struct Cell_window *cell;
	int offset;
	int number;
	struct FE_field *field;
};

/*
Local functions
===============
*/
static int write_FE_node_variants(struct FE_node *node,void *data_void)
/*******************************************************************************
LAST MODIFIED : 22 September 1999

DESCRIPTION :
Iterator function for writing out the variant of each node.
==============================================================================*/
{
	int return_code = 0;
	struct Output_data *output_data;
	struct FE_field_component field_component;
	int variant,node_number;
	char *number,grid_point[50];

	ENTER(write_FE_node_variants);
	if (node && (output_data = (struct Output_data *)data_void))
	{
		/* get the cell_type (variant number) */
		if (field_component.field = FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)(
			"cell_type",(output_data->cell->cell_3d).fe_field_manager))
		{
			field_component.number = 0;
			get_FE_nodal_int_value(node,&field_component,0,FE_NODAL_VALUE,
				&variant);
			/* get the node number */
			GET_NAME(FE_node)(node,&number);
			sscanf(number,"%d",&node_number);
			sprintf(grid_point,"%d\0",node_number-output_data->offset);
			fprintf(output_data->file," Enter collocation point #s/name [EXIT]: ");
			fprintf(output_data->file,"%s\n",grid_point);
			fprintf(output_data->file," The cell variant number is [1]: %d\n",
				variant);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"write_FE_node_variants. "
				"Unable to get the cell_type field");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"write_FE_node_variants. "
			"Invalid arguments");
		return_code = 0;
	}
	LEAVE;
	return(return_code);
} /* END write_FE_node_variants() */

static int write_FE_node_spatially_varying(struct FE_node *node,
	void *data_void)
/*******************************************************************************
LAST MODIFIED : 22 September 1999

DESCRIPTION :
Iterator function for writing out the spatially varying parameters at each
node in the group.
==============================================================================*/
{
	int return_code = 0;
	struct Output_data *output_data;
	int node_number;
	char *number,grid_point[50];
	char *value;

	ENTER(write_FE_node_spatially_varying);
	if (node && (output_data = (struct Output_data *)data_void))
	{
		GET_NAME(FE_node)(node,&number);
		sscanf(number,"%d",&node_number);
		sprintf(grid_point,"%d\0",node_number-output_data->offset);
		fprintf(output_data->file," Enter collocation point #s/name [EXIT]: %s\n",
			grid_point);
		get_FE_nodal_value_as_string(node,output_data->field,0,0,FE_NODAL_VALUE,
			&value);
		fprintf(output_data->file," The value for parameter %d is [ 0.00000D+00]: ",
			output_data->number);
		fprintf(output_data->file,"%s\n",value);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"write_FE_node_spatially_varying. "
			"Invalid arguments");
		return_code = 0;
	}
	LEAVE;
	return(return_code);
} /* END write_FE_node_spatially_varying() */

static void write_real_array_information_using_FE_fields(
	struct Cell_array_information *array,struct Cell_window *cell,
	int number_of_variants,int number_of_parameters,FILE *file)
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Writes out the ipcell information from the array and using field information.
==============================================================================*/
{
	int variant,i;
	struct Cell_array_information *current =
    (struct Cell_array_information *)NULL;
	struct FE_field *field;
	FE_value value;
	float parameter_value;

	ENTER(write_real_array_information_using_FE_fields);
	if ((number_of_parameters > 0) && cell && (number_of_variants > 0))
	{
		for (variant=0;variant<number_of_variants;variant++)
		{
			fprintf(file," Cell variant %d:\n",variant+1);
			current = array;
			for (i=0;i<number_of_parameters;i++)
      {
				/* if the parameter is an indexed field, get its values from the 
					 field */
				if (field = FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)(current->label,
					(cell->cell_3d).fe_field_manager))
				{
					if (get_FE_field_FE_field_type(field) == INDEXED_FE_FIELD)
					{
						if (get_FE_field_FE_value_value(field,variant,&value))
						{
							parameter_value = (float)value;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"write_real_array_information_using_FE_fields. "
								"Unable to get the indexed field value");
							parameter_value = -999999.99;
						}
					}
					else
					{
						parameter_value = (current->value).real_value;
					}
				}
        fprintf(file,"  %3d%s%9.6E \t%s\n",current->number,current->spatial,
          parameter_value,current->label);
        current = current->next;
      } /* for (i) */
		} /* for (variant) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_real_array_information_using_FE_fields. "
			"Invalid arguments");
	}
	LEAVE;
} /* END write_real_array_information_using_FE_fields() */

static void write_array_information_spatially_varying(
	struct Cell_array_information *array,struct Cell_window *cell,
	int number_of_parameters,FILE *file,struct GROUP(FE_node) *node_group,
	int offset)
/*******************************************************************************
LAST MODIFIED : 22 September 1999

DESCRIPTION :
Writes out all parameters in the <array> which are spatially varying. <type>
should be 1 for real and 2 for integer.
==============================================================================*/
{
	int i;
	struct Cell_array_information *current =
    (struct Cell_array_information *)NULL;
	struct FE_field *field;
	struct Output_data output_data;

	ENTER(write_real_array_information_saptially_varying);
	if ((number_of_parameters > 0) && cell)
	{
		current = array;
		for (i=0;i<number_of_parameters;i++)
		{
			/* if the parameter is spatially varying, write out its value
			 at each node */
			if (field = FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)(current->label,
				(cell->cell_3d).fe_field_manager))
			{
				if ((current->spatial[0] == '*') && 
					get_FE_field_FE_field_type(field) == GENERAL_FE_FIELD)
				{
					fprintf(file,"\n");
					fprintf(file," Parameter number %d is [2]:\n",current->number);
					fprintf(file,"   (1) Piecewise constant (defined by elements)\n");
					fprintf(file,"   (2) Piecewise linear (defined by nodes)\n");
					fprintf(file,"   (3) Defined by grid points\n");
					fprintf(file,"    3\n");
					/* write out the nodal values */
					output_data.file = file;
					output_data.cell = cell;
					output_data.offset = offset;
					output_data.number = current->number;
					output_data.field = field;
					FOR_EACH_OBJECT_IN_GROUP(FE_node)(
						write_FE_node_spatially_varying,(void *)(&output_data),
						node_group);
					/* end the parameter data */
					fprintf(file," Enter collocation point #s/name [EXIT]: 0\n");
				}
			}
			current = current->next;
		} /* for (i) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_array_information_spatially_varying. "
			"Invalid arguments");
	}
	LEAVE;
} /* END write_array_information_spatially_varying() */

static void write_integer_array_information_using_FE_fields(
	struct Cell_array_information *array,struct Cell_window *cell,
	int number_of_variants,int number_of_parameters,FILE *file)
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Writes out the ipcell information from the array and using field information.
==============================================================================*/
{
	int variant,i;
	struct Cell_array_information *current =
    (struct Cell_array_information *)NULL;
	struct FE_field *field;
	int value;

	ENTER(write_integer_array_information_using_FE_fields);
	if ((number_of_parameters > 0) && cell && (number_of_variants > 0))
	{
		for (variant=0;variant<number_of_variants;variant++)
		{
			fprintf(file," Cell variant %d:\n",variant+1);
			current = array;
			for (i=0;i<number_of_parameters;i++)
      {
				/* if the parameter is an indexed field, get its values from the 
					 field */
				if (field = FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)(current->label,
					(cell->cell_3d).fe_field_manager))
				{
					if (get_FE_field_FE_field_type(field) == INDEXED_FE_FIELD)
					{
						if (!get_FE_field_int_value(field,variant,&value))
						{
							display_message(ERROR_MESSAGE,
								"write_integer_array_information_using_FE_fields. "
								"Unable to get the indexed field value");
							value = -999999;
						}
					}
					else
					{
						value = (current->value).integer_value;
					}
				}
        fprintf(file,"  %3d%s%d \t%s\n",current->number,current->spatial,
          value,current->label);
        current = current->next;
      } /* for (i) */
		} /* for (variant) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_integer_array_information_using_FE_fields. "
			"Invalid arguments");
	}
	LEAVE;
} /* END write_integer_array_information_using_FE_fields() */

static struct Cell_array_information *add_variables_to_array_information(
  struct Cell_array_information *array_info,struct Cell_variable *variables,
  enum Cell_array array,enum Array_value_type type,int *number_added)
/*******************************************************************************
LAST MODIFIED : 31 May 1999

DESCRIPTION :
Searches through the list of <variables> for array fields matching the given
<array>, and then adds the variable to the list of <array_info> structs.
<array_info> is assumed to point to the start of the list on entry, and new
structures are added BEFORE this in the list. This function returns a pointer
to the start of the list.
==============================================================================*/
{
  struct Cell_variable *current = (struct Cell_variable *)NULL;
  struct Cell_array_information *new = (struct Cell_array_information *)NULL;
  struct Cell_array_information *tmp = (struct Cell_array_information *)NULL;

  ENTER(add_variables_to_array_information);
  *number_added = 0;
  new = array_info;
  tmp = array_info;
  current = variables;
  while (current != (struct Cell_variable *)NULL)
  {
    if (current->array == array)
    {
      /* create the new structure */
      if (ALLOCATE(new,struct Cell_array_information,1))
      {
        if (ALLOCATE(new->label,char,strlen(current->spatial_label)))
        {
          new->type = type;
          new->number = current->position;
          if (current->spatial_switch)
          {
            sprintf(new->spatial,CELL_SPATIALLY_VARYING);
          }
          else
          {
            sprintf(new->spatial,CELL_SPATIALLY_CONSTANT);
          }
          sprintf(new->label,"%s\0",current->spatial_label);
          if (type == ARRAY_VALUE_REAL)
          {
            (new->value).real_value = current->value;
          }
          else
          {
            display_message(WARNING_MESSAGE,
              "add_variables_to_array_information. "
              "non-reals not yet implemented");
          }
          new->next = tmp;
          tmp = new;
          *number_added = *number_added + 1;
        }
        else
        {
          display_message(ERROR_MESSAGE,"add_variables_to_array_information. "
            "Unable to allocate memory for the label");
          *number_added = -1;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"add_variables_to_array_information. "
          "Unable to allocate memory for a new structure");
        *number_added = -1;
      }
    }
    current = current->next;
  }
  LEAVE;
  return (new);
} /* END add_variables_to_array_information() */
  
static struct Cell_array_information *add_parameters_to_array_information(
  struct Cell_array_information *array_info,struct Cell_parameter *parameters,
  enum Cell_array array,enum Array_value_type type,int *number_added)
/*******************************************************************************
LAST MODIFIED : 25 May 1999

DESCRIPTION :
Searches through the list of <parameters> for array fields matching the given
<array>, and then adds the parameter to the list of <array_info> structs.
<array_info> is assumed to point to the start of the list on entry, and new
structures are added BEFORE this in the list. This function returns a pointer
to the start of the list.
==============================================================================*/
{
  struct Cell_parameter *current = (struct Cell_parameter *)NULL;
  struct Cell_array_information *new = (struct Cell_array_information *)NULL;
  struct Cell_array_information *tmp = (struct Cell_array_information *)NULL;

  ENTER(add_parameters_to_array_information);
  *number_added = 0;
  new = array_info;
  tmp = array_info;
  current = parameters;
  while (current != (struct Cell_parameter *)NULL)
  {
    if (current->array == array)
    {
      /* create the new structure */
      if (ALLOCATE(new,struct Cell_array_information,1))
      {
        if (ALLOCATE(new->label,char,strlen(current->spatial_label)))
        {
          new->type = type;
          new->number = current->position;
          if (current->spatial_switch)
          {
            sprintf(new->spatial,CELL_SPATIALLY_VARYING);
          }
          else
          {
            sprintf(new->spatial,CELL_SPATIALLY_CONSTANT);
          }
          sprintf(new->label,"%s\0",current->spatial_label);
          if (type == ARRAY_VALUE_REAL)
          {
            (new->value).real_value = current->value;
          }
          else
          {
            (new->value).integer_value = (int)(current->value);
            /*display_message(WARNING_MESSAGE,
              "add_parameters_to_array_information. "
              "non-reals not yet implemented");*/
          }
          new->next = tmp;
          tmp = new;
          *number_added = *number_added + 1;
        }
        else
        {
          display_message(ERROR_MESSAGE,"add_parameters_to_array_information. "
            "Unable to allocate memory for the label");
          *number_added = -1;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"add_parameters_to_array_information. "
          "Unable to allocate memory for a new structure");
        *number_added = -1;
      }
    }
    current = current->next;
  }
  LEAVE;
  return (new);
} /* END add_parameters_to_array_information() */

#if defined (OLD_CODE)
static int write_variables_to_file(FILE *file,struct Cell_variable *variables)
/*******************************************************************************
LAST MODIFIED : 13 February 1999

DESCRIPTION :
Writes the <variables> to the given <file>
==============================================================================*/
{
  int return_code = 0;
  struct Cell_variable *current = (struct Cell_variable *)NULL;
  char spatial_string[6];

  ENTER(write_variables_to_file);
  if (variables != (struct Cell_variable *)NULL)
  {
    fprintf(file,"<variable-specs>\n");
    current = variables;
    while (current != (struct Cell_variable *)NULL)
    {
      fprintf(file,"  <variable>\n");
      fprintf(file,"    <label>%s</label>\n",current->label);
      if (current->spatial_switch)
      {
        sprintf(spatial_string,"true\0");
      }
      else
      {
        sprintf(spatial_string,"false\0");
      }
      fprintf(file,"    <value units=\"%s\" spatiall-variant=\"%s\">"
        "%f</value>\n",current->units,spatial_string,current->value);
      fprintf(file,"  </variable>\n");
      current = current->next;
    }
    fprintf(file,"</variable-specs>\n");
    return_code = 1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"write_variables_to_file. "
      "No variables to write");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END write_variables_to_file() */
#endif /* defined (OLD_CODE) */

static int write_model_to_file(FILE *file,struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 26 August 1999

DESCRIPTION :
Writes the current model variables and parameters to the given <file>
==============================================================================*/
{
  int return_code = 0;
  struct Cell_variable *current_variable = (struct Cell_variable *)NULL;
  struct Cell_parameter *current_parameter = (struct Cell_parameter *)NULL;
  char spatial_string[6],time_variable_string[6],array_string[15];

  ENTER(write_model_to_file);
  if (cell != (struct Cell_window *)NULL)
  {
    if (cell->current_model)
    {
#if defined (OLD_CODE)
      fprintf(file,"<!--\n%s\n\nThis file contains variables and parameters "
        "for the %s model\n-->\n",VERSION,cell->current_model);
#endif /* defined (OLD_CODE) */
      fprintf(file,"<!--\n\nThis file contains variables and parameters "
        "for the %s model\n-->\n",cell->current_model);
      fprintf(file,"<cell-model>\n");
      /* write out the variables */
      if (cell->variables != (struct Cell_variable *)NULL)
      {
        fprintf(file,"  <variable-specs>\n");
        current_variable = cell->variables;
        while (current_variable != (struct Cell_variable *)NULL)
        {
          fprintf(file,"    <variable>\n");
          switch (current_variable->array)
          {
            case ARRAY_STATE:
            {
              sprintf(array_string,"state\0");
            } break;
            case ARRAY_PARAMETERS:
            {
              sprintf(array_string,"parameters\0");
            } break;
            case ARRAY_PROTOCOL:
            {
              sprintf(array_string,"protocol\0");
            } break;
            default:
            {
              sprintf(array_string,"unkown\0");
            } break;
          }
          fprintf(file,"      <name array=\"%s\" position=\"%d\">%s</name>\n",
            array_string,current_variable->position,
            current_variable->spatial_label);
          fprintf(file,"      <label>%s</label>\n",current_variable->label);
          if (current_variable->spatial_switch)
          {
            sprintf(spatial_string,"true\0");
          }
          else
          {
            sprintf(spatial_string,"false\0");
          }
          fprintf(file,"      <value units=\"%s\" spatially-variant=\"%s\">"
            "%f</value>\n",current_variable->units,spatial_string,
            current_variable->value);
          fprintf(file,"    </variable>\n");
          current_variable = current_variable->next;
        } /* while variables .. */
        fprintf(file,"  </variable-specs>\n");
      } /* if variables ... */
      /* write out the parameters */
      if (cell->parameters != (struct Cell_parameter *)NULL)
      {
        fprintf(file,"  <parameter-specs>\n");
        current_parameter = cell->parameters;
        while (current_parameter != (struct Cell_parameter *)NULL)
        {
          fprintf(file,"    <parameter>\n");
          switch (current_parameter->array)
          {
            case ARRAY_STATE:
            {
              sprintf(array_string,"state\0");
            } break;
            case ARRAY_PARAMETERS:
            {
              sprintf(array_string,"parameters\0");
            } break;
            case ARRAY_PROTOCOL:
            {
              sprintf(array_string,"protocol\0");
            } break;
            default:
            {
              sprintf(array_string,"unkown\0");
            } break;
          }
          fprintf(file,"      <name array=\"%s\" position=\"%d\">%s</name>\n",
            array_string,current_parameter->position,
            current_parameter->spatial_label);
          fprintf(file,"      <label>%s</label>\n",current_parameter->label);
          if (current_parameter->spatial_switch)
          {
            sprintf(spatial_string,"true\0");
          }
          else
          {
            sprintf(spatial_string,"false\0");
          }
          if (current_parameter->time_variable_allowed)
          {
            if (current_parameter->time_variable_switch)
            {
              sprintf(time_variable_string,"true\0");
            }
            else
            {
              sprintf(time_variable_string,"false\0");
            }
          }
          else
          {
            sprintf(time_variable_string,"false\0");
          }
          fprintf(file,"      <value units=\"%s\" spatially-variant=\"%s\""
            "time-variable=\"%s\">"
            "%f</value>\n",current_parameter->units,spatial_string,
            time_variable_string,current_parameter->value);
          fprintf(file,"    </parameter>\n");
          current_parameter = current_parameter->next;
        } /* while parameters .. */
        fprintf(file,"  </parameter-specs>\n");
      } /* if parameters ... */
      fprintf(file,"</cell-model>\n");
      return_code = 1;
    }
    else
    {
      display_message(ERROR_MESSAGE,"write_model_to_file. "
        "No model is currently set");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"write_model_to_file. "
      "Missing Cell window");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END write_model_to_file() */

#if defined (OLD_CODE) /* the old format */
static int write_to_cmiss_file(FILE *file,struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 22 May 1999

DESCRIPTION :
Writes the current model variables and parameters to the given CMISS <file>
==============================================================================*/
{
  int return_code = 0,model_id,num,i;
  struct Cell_variable *current_variable = (struct Cell_variable *)NULL;
  struct Cell_parameter *current_parameter = (struct Cell_parameter *)NULL;
  char spatial_string[6];

  ENTER(write_to_cmiss_file);
  if (cell != (struct Cell_window *)NULL)
  {
    if (!strncmp(cell->current_model,"Hodg",2))
    {
      model_id = 5;
    }
    else if (!strncmp(cell->current_model,"Luo",2))
    {
      model_id = 2;
    }
    /* write out file heading etc.. */
    fprintf(file," CMISS Version 1.21 ipcell File Version 0\n");
    fprintf(file," Heading: .ipcell file generated by CELL for the %s model"
      "\n\n",cell->current_model);
    /* write out the model types and control modes */
    fprintf(file,"  (modl001) %3d\n",model_id); /* membrane model */
    fprintf(file,"  (modl002) %3d\n",0); /* mechanics model */
    fprintf(file,"  (modl003) %3d\n",0); /* metabolism */
    fprintf(file,"  (modl004) %3d\n",0); /* drug */
    fprintf(file,"  (modl005) %3d\n",0);
    fprintf(file,"  (modl006) %3d\n",0);
    fprintf(file,"  (modl007) %3d\n",0);
    fprintf(file,"  (modl008) %3d\n\n",0);
    /* count the number of parameters */
    current_parameter = cell->parameters;
    num = 0;
    while (current_parameter != (struct Cell_parameter *)NULL)
    {
      num++;
      current_parameter = current_parameter->next;
    }
    /* write out the parameters */
    if (num > 0)
    {
      fprintf(file,"Number of electrical real parameters: %3d\n",num);
      current_parameter = cell->parameters;
      i = 0;
      while (current_parameter != (struct Cell_parameter *)NULL)
      {
        i++;
        fprintf(file,"  (eleR%03d) %12.5E\n",i,current_parameter->value);
        current_parameter = current_parameter->next;
      }
      fprintf(file,"\n");
    }
    /* count the number of variables */
    current_variable = cell->variables;
    num = 0;
    while (current_variable != (struct Cell_variable *)NULL)
    {
      num++;
      current_variable = current_variable->next;
    }
    /* write out the variables */
    if (num > 0)
    {
      fprintf(file,"Number of initial values: %3d\n",num);
      current_variable = cell->variables;
      i = 0;
      while (current_variable != (struct Cell_variable *)NULL)
      {
        i++;
        fprintf(file," (y%03d) %12.5E\n",i,current_variable->value);
        current_variable = current_variable->next;
      }
      fprintf(file,"\n");
    }
    return_code = 1;
  }
  else
  {
    return_code = 0;
    display_message(ERROR_MESSAGE,"write_to_cmiss_file. "
      "Missing cell window");
  }
  LEAVE;
  return(return_code);
} /* END write_to_cmiss_file() */
#endif /* defined (OLD_CODE) */

static int write_to_cmiss_file(FILE *file,struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 25 May 1999

DESCRIPTION :
Writes the current model variables and parameters to the given CMISS <file>
==============================================================================*/
{
  int return_code = 0,i,model_id,num_state,num_parameters,num_protocol,
    num_ari,num_aro,num_aio,num_aii,num_model,num_control,num_derived;
  struct Cell_output *current_output = (struct Cell_output *)NULL;
  struct Cell_array_information *state = (struct Cell_array_information *)NULL;
  struct Cell_array_information *derived =
    (struct Cell_array_information *)NULL;
  struct Cell_array_information *model = (struct Cell_array_information *)NULL;
  struct Cell_array_information *control =
    (struct Cell_array_information *)NULL;
  struct Cell_array_information *parameter =
    (struct Cell_array_information *)NULL;
  struct Cell_array_information *protocol =
    (struct Cell_array_information *)NULL;
  struct Cell_array_information *aii = (struct Cell_array_information *)NULL;
  struct Cell_array_information *aio = (struct Cell_array_information *)NULL;
  struct Cell_array_information *ari = (struct Cell_array_information *)NULL;
  struct Cell_array_information *aro = (struct Cell_array_information *)NULL;
  struct Cell_array_information *current =
    (struct Cell_array_information *)NULL;

  ENTER(write_to_cmiss_file);
  USE_PARAMETER(aro);
  USE_PARAMETER(aio);
  USE_PARAMETER(derived);
  if (cell != (struct Cell_window *)NULL)
  {
    if (!strncmp(cell->current_model,"Hodg",2))
    {
      model_id = 5;
    }
    else if (!strncmp(cell->current_model,"Luo",2))
    {
      model_id = 2;
    }
    USE_PARAMETER(model_id);
    /* loop through all parameters and variables, setting up the computational
			 array information */
    num_state = 0;
    num_derived = 0;
    num_model = 0;
    num_control = 0;
    num_parameters = 0;
    num_protocol = 0;
    num_aii = 0;
    num_aio = 0;
    num_ari = 0;
    num_aro = 0;
    /* assume all variables are the only state variables ?? */
    state = add_variables_to_array_information(state,cell->variables,
      ARRAY_STATE,ARRAY_VALUE_REAL,&num_state);
    /* assume parameters always fall into these categories ?? */
    parameter = add_parameters_to_array_information(parameter,cell->parameters,
      ARRAY_PARAMETERS,ARRAY_VALUE_REAL,&num_parameters);
    protocol = add_parameters_to_array_information(protocol,cell->parameters,
      ARRAY_PROTOCOL,ARRAY_VALUE_REAL,&num_protocol);
    model = add_parameters_to_array_information(model,cell->parameters,
      ARRAY_MODEL,ARRAY_VALUE_INTEGER,&num_model);
    control = add_parameters_to_array_information(control,cell->parameters,
      ARRAY_CONTROL,ARRAY_VALUE_INTEGER,&num_control);
    aii = add_parameters_to_array_information(aii,cell->parameters,
      ARRAY_AII,ARRAY_VALUE_INTEGER,&num_aii);
    ari = add_parameters_to_array_information(ari,cell->parameters,
      ARRAY_ARI,ARRAY_VALUE_REAL,&num_ari);
    /* count the number of outputs */
    current_output = cell->outputs;
    while (current_output != (struct Cell_output *)NULL)
    {
      num_derived++;
      current_output = current_output->next;
    }
    /* subtract the number of state variables to get the number of "derived"
			 output variables */
    num_derived -= num_state;
    /* now write out the file */
    /* write out file heading etc.. */
    fprintf(file," CMISS Version 1.21 ipcell File Version 1\n");
    fprintf(file," Heading: ipcell file generated by CELL for the %s model"
      "\n\n",cell->current_model);
    fprintf(file," The number of cell model variants is: 1\n");
    fprintf(file," The number of state variables is: %d\n",num_state);
    fprintf(file," The number of ODE variables is: %d\n",num_state);
    fprintf(file," The number of derived variables is: %d\n",num_derived);
    fprintf(file," The number of cell model parameters is: %d\n",num_model);
    fprintf(file," The number of cell control parameters is: %d\n",num_control);
    fprintf(file," The number of cell material parameters is: %d\n",
      num_parameters);
    fprintf(file," The number of cell protocol parameters is: %d\n",
      num_protocol);
    fprintf(file," The number of additional integer input parameters is: %d\n",
      num_aii);
    fprintf(file," The number of additional integer output parameters is: %d\n",
      num_aio);
    fprintf(file," The number of additional real input parameters is: %d\n",
      num_ari);
    fprintf(file," The number of additional real output parameters is: %d\n",
      num_aro);
    if (num_state > 0)
    {
      fprintf(file,"\n");
      fprintf(file," State variables:\n");
      fprintf(file," Cell variant 1:\n");
      current = state;
      for (i=0;i<num_state;i++)
      {
        fprintf(file,"  %3d%s%9.6E \t%s\n",current->number,current->spatial,
          (current->value).real_value,current->label);
        current = current->next;
      }
    }
    if (num_model > 0)
    {
      fprintf(file,"\n");
      fprintf(file," Model variables:\n");
      fprintf(file," Cell variant 1:\n");
      current = model;
      for (i=0;i<num_model;i++)
      {
        fprintf(file,"  %3d%s%d \t%s\n",current->number,current->spatial,
          (current->value).integer_value,current->label);
        current = current->next;
      }
    }
    if (num_control > 0)
    {
      fprintf(file,"\n");
      fprintf(file," Control variables:\n");
      fprintf(file," Cell variant 1:\n");
      current = control;
      for (i=0;i<num_control;i++)
      {
        fprintf(file,"  %3d%s%d \t%s\n",current->number,current->spatial,
          (current->value).integer_value,current->label);
        current = current->next;
      }
    }
    if (num_parameters > 0)
    {
      fprintf(file,"\n");
      fprintf(file," Parameter variables:\n");
      fprintf(file," Cell variant 1:\n");
      current = parameter;
      for (i=0;i<num_parameters;i++)
      {
        fprintf(file,"  %3d%s%9.6E \t%s\n",current->number,current->spatial,
          (current->value).real_value,current->label);
        current = current->next;
      }
    }
    if (num_protocol > 0)
    {
      fprintf(file,"\n");
      fprintf(file," Protocol variables:\n");
      fprintf(file," Cell variant 1:\n");
      current = protocol;
      for (i=0;i<num_protocol;i++)
      {
        fprintf(file,"  %3d%s%9.6E \t%s\n",current->number,current->spatial,
          (current->value).real_value,current->label);
        current = current->next;
      }
    }
    if (num_aii > 0)
    {
      fprintf(file,"\n");
      fprintf(file," Additional integer input variables:\n");
      fprintf(file," Cell variant 1:\n");
      current = aii;
      for (i=0;i<num_aii;i++)
      {
        fprintf(file,"  %3d%s%d \t%s\n",current->number,current->spatial,
          (current->value).integer_value,current->label);
        current = current->next;
      }
    }
    if (num_ari > 0)
    {
      fprintf(file,"\n");
      fprintf(file," Additional real input variables:\n");
      fprintf(file," Cell variant 1:\n");
      current = ari;
      for (i=0;i<num_ari;i++)
      {
        fprintf(file,"  %3d%s%9.6E \t%s\n",current->number,current->spatial,
          (current->value).real_value,current->label);
        current = current->next;
      }
    }

    return_code = 1;
  }
  else
  {
    return_code = 0;
    display_message(ERROR_MESSAGE,"write_to_cmiss_file. "
      "Missing cell window");
  }
  LEAVE;
  return(return_code);
} /* END write_to_cmiss_file() */

static int write_time_variable_to_file(struct Control_curve *variable,
  FILE *file,char *name)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Write the given <variable> to the specified <file>.
==============================================================================*/
{
  float end_time,scale_factor,comp_min,comp_max,t_grid,c_grid,start_time;
  float node_time,*values;
  int return_code = 0,element_no,node_no,comp_no,number_of_components;
  int number_of_elements,nodes_per_element,number_of_derivs;
  
  ENTER(write_time_variable_to_file);
  /* check arguments */
  if (variable)
  {
    fprintf(file,"%s\n",name);
    fprintf(file,"  basis type : ");
    switch (Control_curve_get_fe_basis_type(variable))
    {
      case CUBIC_HERMITE:
      {
        fprintf(file,"cubic_hermite");
      } break;
      case LINEAR_LAGRANGE:
      {
        fprintf(file,"linear_lagrange");
      } break;
      case QUADRATIC_LAGRANGE:
      {
        fprintf(file,"quadratic_lagrange");
      } break;
      case CUBIC_LAGRANGE:
      {
        fprintf(file,"cubic_lagrange");
      } break;
      default:
      {
        fprintf(file,"unknown");
      } break;
    } /* switch (variable->fe_basis_type) */
    fprintf(file,"\n");
    number_of_components = Control_curve_get_number_of_components(variable);
    fprintf(file,"  number of components : %d",number_of_components);
    if (number_of_components > 1)
    {
      fprintf(file," (only the first component is used)");
    }
    fprintf(file,"\n");
    for (comp_no=0;comp_no<number_of_components;comp_no++)
    {
      Control_curve_get_edit_component_range(variable,comp_no,
				&comp_min,&comp_max);
      fprintf(file,"    component %d range : %g to %g\n",comp_no+1,comp_min,
        comp_max);
    }
		Control_curve_get_parameter_grid(variable,&t_grid);
		Control_curve_get_value_grid(variable,&c_grid);
    fprintf(file,"  component grid size : %g\n",c_grid);
    fprintf(file,"  time grid size : %g\n",t_grid);
    Control_curve_get_parameter_range(variable,&start_time,&end_time);
    fprintf(file,"  start time : %g\n",start_time);
    return_code = 1;
    values = (float *)NULL;
    number_of_elements = Control_curve_get_number_of_elements(variable);
    fprintf(file,"  number of elements : %d\n",number_of_elements);
    for (element_no=1;return_code&&(element_no<=number_of_elements);
         element_no++)
    {
      fprintf(file,"  element %d :\n",element_no);
      nodes_per_element = Control_curve_get_nodes_per_element(variable);
      number_of_derivs = Control_curve_get_derivatives_per_node(variable);
      for (node_no=0;return_code&&(node_no<nodes_per_element);node_no++)
      {
				if (ALLOCATE(values,FE_value,
					Control_curve_get_number_of_components(variable)))
				{
					if (Control_curve_get_node_values(variable,element_no,node_no,
						values)&&
						Control_curve_get_parameter(variable,element_no,node_no,&node_time))
					{
						fprintf(file,"    node %d %d (time = %g) : coordinates",
							element_no,node_no+1,node_time);
						for (comp_no=0;comp_no<number_of_components;comp_no++)
						{
							fprintf(file," %g",values[comp_no]);
						}
						if (number_of_derivs > 0)
						{
							if (Control_curve_get_node_derivatives(variable,element_no,
								node_no,values))
							{
								fprintf(file," derivatives");
								for (comp_no=0;comp_no<number_of_components;comp_no++)
								{
									fprintf(file," %g",values[comp_no]);
								}
								if (Control_curve_get_scale_factor(variable,element_no,
									node_no,&scale_factor))
								{
									fprintf(file," scale factor %g",scale_factor);
								}
								else
								{
									return_code = 0;
									display_message(ERROR_MESSAGE,"write_time_variable_to_file. "
										"can not get node scale factor");
								}
							}
							else
							{
								return_code = 0;
								display_message(ERROR_MESSAGE,"write_time_variable_to_file. "
									"can not get node derivs");
							}
						}
					}
					else
					{
						return_code = 0;
						display_message(ERROR_MESSAGE,"write_time_variable_to_file. "
							"can not get node coords or time");
					}
					DEALLOCATE(values);
        }
        else
        {
          return_code = 0;
          display_message(ERROR_MESSAGE,"write_time_variable_to_file.  "
            "Not enough memory");
        }
        fprintf(file,"\n");
      }/* for (node_no) */
    }/* for (element_no) */
  }
  else
  {
    return_code = 0;
    display_message(ERROR_MESSAGE,"write_time_variable_to_file. "
      "Invalid argument");
  }
  LEAVE;
  return (return_code);
} /* END write_time_variable_to_file() */

/*
Global functions
================
*/
#if defined (OLD_CODE)
int write_variables_file(char *filename,XtPointer cell_window)
/*******************************************************************************
LAST MODIFIED : 13 February 1999

DESCRIPTION :
The function called when a variables file is selected via the file selection
dialog box, file -> write -> variables file.
==============================================================================*/
{
  int return_code = 0;
  struct Cell_window *cell = (struct Cell_window *)NULL;
  FILE *output;

  ENTER(write_variables_file);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (output = fopen(filename,"w"))
    {
      return_code = write_variables_to_file(output,cell->variables);
      fclose(output);
    }
    else
    {
      display_message(ERROR_MESSAGE,"write_variables_file. "
        "Unable to open file - %s",filename);
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"write_variables_file. "
      "Missing Cell window");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END write_variables_file() */
#endif /* defined (OLD_CODE) */

int write_model_file(char *filename,XtPointer cell_window)
/*******************************************************************************
LAST MODIFIED : 28 February 1999

DESCRIPTION :
The function called when a model file is selected via the file selection
dialog box, file -> write -> model file.
==============================================================================*/
{
  int return_code = 0;
  struct Cell_window *cell = (struct Cell_window *)NULL;
  FILE *output;

  ENTER(write_model_file);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (output = fopen(filename,"w"))
    {
      return_code = write_model_to_file(output,cell);
      fclose(output);
    }
    else
    {
      display_message(ERROR_MESSAGE,"write_model_file. "
        "Unable to open file - %s",filename);
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"write_model_file. "
      "Missing Cell window");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END write_model_file() */

int write_cmiss_file(char *filename,XtPointer cell_window)
/*******************************************************************************
LAST MODIFIED : 12 May 1999

DESCRIPTION :
The function called when a cmiss file is selected via the file selection
dialog box, file -> write -> cmiss file.
==============================================================================*/
{
  int return_code = 0;
  struct Cell_window *cell = (struct Cell_window *)NULL;
  FILE *output;

  ENTER(write_cmiss_file);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (output = fopen(filename,"w"))
    {
      return_code = write_to_cmiss_file(output,cell);
      fclose(output);
    }
    else
    {
      display_message(ERROR_MESSAGE,"write_cmiss_file. "
        "Unable to open file - %s",filename);
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"write_cmiss_file. "
      "Missing Cell window");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END write_cmiss_file() */

int write_time_variable_file(char *filename,XtPointer cell_window)
/*******************************************************************************
LAST MODIFIED : 9 November 1999

DESCRIPTION :
The function called when a time variable file is selected via the file selection
dialog box, file -> write -> time variables file.
==============================================================================*/
{
  int return_code = 0;
  struct Cell_window *cell = (struct Cell_window *)NULL;
  struct Cell_variable *variables = (struct Cell_variable *)NULL;
  struct Control_curve *variable = (struct Control_curve *)NULL;
  char *name;
  FILE *output;

  ENTER(write_time_variable_file);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (output = fopen(filename,"w"))
    {
      /* find the time variable */
      variables = cell->variables;
      while (variables != (struct Cell_variable *)NULL)
      {
        if (variables->control_curve != (struct Control_curve *)NULL)
        {
          variable = variables->control_curve;
          name = variables->spatial_label;
        }
        variables = variables->next;
      } /* while variables */
      if (variable != (struct Control_curve *)NULL)
      {
        return_code = write_time_variable_to_file(variable,output,name);
      }
      else
      {
        display_message(INFORMATION_MESSAGE,"No time variables to write out\n");
        return_code = 1;
      }
      fclose(output);
    }
    else
    {
      display_message(ERROR_MESSAGE,"write_time_variable_file. "
        "Unable to open file - %s",filename);
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"write_time_variable_file. "
      "Missing Cell window");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END write_time_variable_file() */

int export_FE_node_to_ipcell(char *filename,struct FE_node *node,
  struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Exports parameter and variable fields from the <node> to the ipcell file given
by <filename>.
==============================================================================*/
{
  int return_code = 0;
	int num_state,num_parameters,num_protocol,num_ari,num_aro,num_aio,num_aii,
		num_model,num_control,num_derived;
  struct Cell_array_information *state = (struct Cell_array_information *)NULL;
  struct Cell_array_information *derived =
    (struct Cell_array_information *)NULL;
  struct Cell_array_information *model = (struct Cell_array_information *)NULL;
  struct Cell_array_information *control =
    (struct Cell_array_information *)NULL;
  struct Cell_array_information *parameter =
    (struct Cell_array_information *)NULL;
  struct Cell_array_information *protocol =
    (struct Cell_array_information *)NULL;
  struct Cell_array_information *aii = (struct Cell_array_information *)NULL;
  struct Cell_array_information *aio = (struct Cell_array_information *)NULL;
  struct Cell_array_information *ari = (struct Cell_array_information *)NULL;
  struct Cell_array_information *aro = (struct Cell_array_information *)NULL;
  FILE *file;
	struct FE_field_component field_component;
	int number_of_variants = 0;

  ENTER(export_FE_node_to_ipcell);
  USE_PARAMETER(aro);
  USE_PARAMETER(aio);
  USE_PARAMETER(derived);
  if (cell && filename && node && (file = fopen(filename,"w")))
  {
		if (check_model_id(cell,node))
		{
			/* find an indexed field, and get its number of values - this will
				 always give the number of variants ??? */
			if (field_component.field = FIRST_OBJECT_IN_MANAGER_THAT(FE_field)(
				check_field_type,(void *)INDEXED_FE_FIELD,
				(cell->cell_3d).fe_field_manager))
			{
				number_of_variants = 
					get_FE_field_number_of_values(field_component.field);
			}
			else
			{
				display_message(WARNING_MESSAGE,"export_FE_node_to_ipcell. "
					"Unable to find an indexed field, assuming 1 variant");
				number_of_variants = 1;
			}
			if (number_of_variants > 0)
			{
				/* now sort the parameters into their arrays */
				num_state = 0;
				num_derived = 0;
				num_model = 0;
				num_control = 0;
				num_parameters = 0;
				num_protocol = 0;
				num_aii = 0;
				num_aio = 0;
				num_ari = 0;
				num_aro = 0;
				/* assume all variables are the only state variables ?? */
				state = add_variables_to_array_information(state,cell->variables,
					ARRAY_STATE,ARRAY_VALUE_REAL,&num_state);
				/* assume parameters always fall into these categories ?? */
				parameter = add_parameters_to_array_information(parameter,
					cell->parameters,ARRAY_PARAMETERS,ARRAY_VALUE_REAL,&num_parameters);
				protocol = add_parameters_to_array_information(protocol,
					cell->parameters,ARRAY_PROTOCOL,ARRAY_VALUE_REAL,&num_protocol);
				model = add_parameters_to_array_information(model,cell->parameters,
					ARRAY_MODEL,ARRAY_VALUE_INTEGER,&num_model);
				control = add_parameters_to_array_information(control,cell->parameters,
					ARRAY_CONTROL,ARRAY_VALUE_INTEGER,&num_control);
				aii = add_parameters_to_array_information(aii,cell->parameters,
					ARRAY_AII,ARRAY_VALUE_INTEGER,&num_aii);
				ari = add_parameters_to_array_information(ari,cell->parameters,
					ARRAY_ARI,ARRAY_VALUE_REAL,&num_ari);
				/* now write out the file */
				/* write out file heading etc.. */
				fprintf(file," CMISS Version 1.21 ipcell File Version 1\n");
				fprintf(file," Heading: ipcell file generated by CELL for the %s model"
					"\n\n",cell->current_model);
				fprintf(file," The number of cell model variants is: %d\n",
					number_of_variants);
				fprintf(file," The number of state variables is: %d\n",num_state);
				fprintf(file," The number of ODE variables is: %d\n",num_state);
				fprintf(file," The number of derived variables is: %d\n",num_derived);
				fprintf(file," The number of cell model parameters is: %d\n",num_model);
				fprintf(file," The number of cell control parameters is: %d\n",
					num_control);
				fprintf(file," The number of cell material parameters is: %d\n",
					num_parameters);
				fprintf(file," The number of cell protocol parameters is: %d\n",
					num_protocol);
				fprintf(file," The number of additional integer input parameters "
					"is: %d\n",num_aii);
				fprintf(file," The number of additional integer output parameters "
					"is: %d\n",num_aio);
				fprintf(file," The number of additional real input parameters is: %d\n",
					num_ari);
				fprintf(file," The number of additional real output parameters "
					"is: %d\n",num_aro);
				if (num_state > 0)
				{
					fprintf(file,"\n");
					fprintf(file," State variables:\n");
					write_real_array_information_using_FE_fields(state,cell,
						number_of_variants,num_state,file);
				}
				if (num_model > 0)
				{
					fprintf(file,"\n");
					fprintf(file," Model variables:\n");
					write_integer_array_information_using_FE_fields(model,cell,
						number_of_variants,num_model,file);
				}
				if (num_control > 0)
				{
					fprintf(file,"\n");
					fprintf(file," Control variables:\n");
					write_integer_array_information_using_FE_fields(control,cell,
						number_of_variants,num_control,file);
				}
				if (num_parameters > 0)
				{
					fprintf(file,"\n");
					fprintf(file," Parameter variables:\n");
					write_real_array_information_using_FE_fields(parameter,cell,
						number_of_variants,num_parameters,file);
				}
				if (num_protocol > 0)
				{
					fprintf(file,"\n");
					fprintf(file," Protocol variables:\n");
					write_real_array_information_using_FE_fields(protocol,cell,
						number_of_variants,num_protocol,file);
				}
				if (num_aii > 0)
				{
					fprintf(file,"\n");
					fprintf(file," Additional integer input variables:\n");
					write_integer_array_information_using_FE_fields(aii,cell,
						number_of_variants,num_aii,file);
				}
				if (num_ari > 0)
				{
					fprintf(file,"\n");
					fprintf(file," Additional real input variables:\n");
					write_real_array_information_using_FE_fields(ari,cell,
						number_of_variants,num_ari,file);
				}
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,"export_FE_node_to_ipcell. "
					"Can not get the number of variants.");
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"export_FE_node_to_ipcell. "
				"Model ID's do not match.");
			return_code = 0;
		}
		fclose(file);
  }
  else
  {
		display_message(ERROR_MESSAGE,"export_FE_node_to_ipcell. "
			"Invalid arguments");
		return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END export_FE_node_to_ipcell() */

int export_FE_node_group_to_ipmatc(char *filename,
	struct GROUP(FE_node) *node_group,struct Cell_window *cell,
	struct FE_node *first_node,int offset)
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Exports the spatially varying parameters to a ipmatc file, from the node group.
<offset> specifies the offset of the grid point number from the node number.
==============================================================================*/
{
	int return_code = 0,num_parameters;
	FILE *file;
	struct Output_data output_data;
  struct Cell_array_information *array_information = 
		(struct Cell_array_information *)NULL;

	ENTER(export_FE_node_group_to_ipmatc);
	if (cell && filename && node_group && first_node && 
		(file = fopen(filename,"w")))
  {
		if (check_model_id(cell,first_node))
		{
			/* write out the header, etc.. */
			fprintf(file," CMISS Version 1.21 ipmatc File Version 2\n");
			fprintf(file," Heading: ipmatc file generated by CELL\n");
			fprintf(file,"\n");
			fprintf(file," Enter the cell variant for each collocation point:\n");
			/* write out the variant for each node in the group */
			output_data.cell = cell;
			output_data.file = file;
			output_data.offset = offset;
			FOR_EACH_OBJECT_IN_GROUP(FE_node)(write_FE_node_variants,
				(void *)(&output_data),node_group);
			/* end the variant data */
			fprintf(file," Enter collocation point #s/name [EXIT]: 0\n");
			fprintf(file,"\n");
			/* now loop through each of the arrays and write out the spatially
				 parameters */
			fprintf(file," State variables:\n");
			num_parameters = 0;
			array_information = add_variables_to_array_information(array_information,
				cell->variables,ARRAY_STATE,ARRAY_VALUE_REAL,&num_parameters);
			if (num_parameters > 0)
			{
				write_array_information_spatially_varying(array_information,cell,
					num_parameters,file,node_group,offset);
			}
			DEALLOCATE(array_information);
			array_information = (struct Cell_array_information *)NULL;
			num_parameters = 0;
			fprintf(file,"\n");
			fprintf(file," Model variables:\n");
			array_information = add_parameters_to_array_information(array_information,
				cell->parameters,ARRAY_MODEL,ARRAY_VALUE_INTEGER,&num_parameters);
			if (num_parameters > 0)
			{
				write_array_information_spatially_varying(array_information,cell,
					num_parameters,file,node_group,offset);
			}
			DEALLOCATE(array_information);
			array_information = (struct Cell_array_information *)NULL;
			num_parameters = 0;			
			fprintf(file,"\n");
			fprintf(file," Control variables:\n");
			array_information = add_parameters_to_array_information(array_information,
				cell->parameters,ARRAY_CONTROL,ARRAY_VALUE_INTEGER,&num_parameters);
			if (num_parameters > 0)
			{
				write_array_information_spatially_varying(array_information,cell,
					num_parameters,file,node_group,offset);
			}
			DEALLOCATE(array_information);
			array_information = (struct Cell_array_information *)NULL;
			num_parameters = 0;			
			fprintf(file,"\n");
			fprintf(file," Parameter variables:\n");
			array_information = add_parameters_to_array_information(array_information,
				cell->parameters,ARRAY_PARAMETERS,ARRAY_VALUE_REAL,&num_parameters);
			if (num_parameters > 0)
			{
				write_array_information_spatially_varying(array_information,cell,
					num_parameters,file,node_group,offset);
			}
			DEALLOCATE(array_information);
			array_information = (struct Cell_array_information *)NULL;
			num_parameters = 0;			
			fprintf(file,"\n");
			fprintf(file," Protocol variables:\n");
			array_information = add_parameters_to_array_information(array_information,
				cell->parameters,ARRAY_PROTOCOL,ARRAY_VALUE_REAL,&num_parameters);
			if (num_parameters > 0)
			{
				write_array_information_spatially_varying(array_information,cell,
					num_parameters,file,node_group,offset);
			}
			DEALLOCATE(array_information);
			array_information = (struct Cell_array_information *)NULL;
			num_parameters = 0;			
			fprintf(file,"\n");
			fprintf(file," Additional integer input variables:\n");
			array_information = add_parameters_to_array_information(array_information,
				cell->parameters,ARRAY_AII,ARRAY_VALUE_INTEGER,&num_parameters);
			if (num_parameters > 0)
			{
				write_array_information_spatially_varying(array_information,cell,
					num_parameters,file,node_group,offset);
			}
			DEALLOCATE(array_information);
			array_information = (struct Cell_array_information *)NULL;
			num_parameters = 0;			
			fprintf(file,"\n");
			fprintf(file," Additional real input variables:\n");
			array_information = add_parameters_to_array_information(array_information,
				cell->parameters,ARRAY_ARI,ARRAY_VALUE_REAL,&num_parameters);
			if (num_parameters > 0)
			{
				write_array_information_spatially_varying(array_information,cell,
					num_parameters,file,node_group,offset);
			}
			DEALLOCATE(array_information);
			array_information = (struct Cell_array_information *)NULL;
			num_parameters = 0;			
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"export_FE_node_group_to_ipmatc. "
				"Model ID's do not match.");
			return_code = 0;
		}
		fclose(file);
	}
	else
	{
		display_message(ERROR_MESSAGE,"export_FE_node_group_to_ipmatc. "
			"Invalid arguments");
		return_code = 0;
	}
	LEAVE;
	return(return_code);
} /* END export_FE_node_group_to_ipmatc() */
