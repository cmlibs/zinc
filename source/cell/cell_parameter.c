/*******************************************************************************
FILE : cell_parameter.c

LAST MODIFIED : 28 August 2000

DESCRIPTION :
Functions and structures for using the Cell_parameter structure.
==============================================================================*/
#include <stdio.h>
#include <string.h>
#if defined (MOTIF)
#include <Xm/Xm.h>
#include <Xm/RowColumn.h>
#include <Xm/Label.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include <Xm/TextF.h>
#endif /* if defined (MOTIF) */
#include "user_interface/user_interface.h"
#include "general/debug.h"
#include "cell/cell_window.h"
/*#include "cell/parameters_dialog.h"*/
#include "cell/cell_parameter.h"
#include "cell/input.h"

/*
Module types
============
*/

/*
Module variables
================
*/

/*
Module functions
================
*/

/*
Global functions
================
*/
int set_parameter_information(struct Cell_window *cell,char *array,
  char *position,char *name,char *label,char *units,char *spatial,
  char *control_curve,char *value,int default_value)
/*******************************************************************************
LAST MODIFIED : 28 August 2000

DESCRIPTION :
Sets the information in the parameter structure for use when creating the
parameters dialog. The <name> is used as the label for the spatial toggle.
If <default_value> is 0, then the parameter is not a default value, and MUST
replace one in the list, if it does not correspond to a parameter already in
the list, then it is ignored.

<array> specifies the computational array the parameter is associated with,
and <array_position> gives the parameter's position in that array. Both are
used when writing out ipcell files.

?? Assume that the default values are always read in first, which sets all
parameter values and their order, so that when another set of parameters is
read in from a model file the existing parameters will simply be replaced by
the newer ones, keeping the order the same. ??
==============================================================================*/
{
  int return_code = 0,found,i;
  struct Cell_parameter *new = (struct Cell_parameter *)NULL;
  struct Cell_parameter *current = (struct Cell_parameter *)NULL;
  struct Cell_parameter *temp = (struct Cell_parameter *)NULL;

  ENTER(set_parameter_information);
  if (cell)
  {
    if (ALLOCATE(new,struct Cell_parameter,1))
    {
      /* initialise */
      new->label = (char *)NULL;
      new->units = (char *)NULL;
      new->spatial_label = (char *)NULL;
      new->array = ARRAY_UNKNOWN;
      new->position = 0;
      new->spatial_switch = 0;
      new->control_curve[0] = (struct Control_curve *)NULL;
      new->control_curve[1] = (struct Control_curve *)NULL;
      new->control_curve[2] = (struct Control_curve *)NULL;
      new->edit_distributed = &((cell->distributed).edit);
      if (default_value)
      {
        new->control_curve_allowed = 0;
      }
      new->control_curve_switch = 0;
      new->value = 0.0;
      new->number_of_components = 0;
      /*new->components = (enum Cell_components *)NULL;*/
      new->components = (char **)NULL;
      new->next = (struct Cell_parameter *)NULL;
      if ((ALLOCATE(new->label,char,strlen(label)+1)) &&
        (ALLOCATE(new->units,char,strlen(units)+1)) &&
        (ALLOCATE(new->spatial_label,char,strlen(name)+1)))
      {
				strcpy(new->label,label);
        strcpy(new->units,units);
        strcpy(new->spatial_label,name);
        if (!strncmp(array,"state",strlen("state")))
        {
          new->array = ARRAY_STATE;
        }
        else if (!strncmp(array,"derived",strlen("derived")))
        {
          new->array = ARRAY_DERIVED;
        }
        else if (!strncmp(array,"parameters",strlen("parameters")))
        {
          new->array = ARRAY_PARAMETERS;
        }
        else if (!strncmp(array,"protocol",strlen("protocol")))
        {
          new->array = ARRAY_PROTOCOL;
        }
        else if (!strncmp(array,"control",strlen("control")))
        {
          new->array = ARRAY_CONTROL;
        }
        else if (!strncmp(array,"model",strlen("model")))
        {
          new->array = ARRAY_MODEL;
        }
        else if (!strncmp(array,"aii",strlen("aii")))
        {
          new->array = ARRAY_AII;
        }
        else if (!strncmp(array,"aio",strlen("aio")))
        {
          new->array = ARRAY_AIO;
        }
        else if (!strncmp(array,"ari",strlen("ari")))
        {
          new->array = ARRAY_ARI;
        }
        else if (!strncmp(array,"aro",strlen("aro")))
        {
          new->array = ARRAY_ARO;
        }
        else
        {
          new->array = ARRAY_UNKNOWN;
        }
        sscanf(position,"%d",&(new->position));
        if (!strncmp(spatial,"true",strlen("true")))
        {
          new->spatial_switch = 1;
        }
        else
        {
          new->spatial_switch = 0;
        }
        if (default_value)
        {
          if (!strncmp(control_curve,"true",strlen("true")))
          {
            new->control_curve_allowed = 1;
          }
          else
          {
            new->control_curve_allowed = 0;
          }
        }
        sscanf(value,"%f",&(new->value));
        if (!strcmp(new->spatial_label,"tstart"))
        {
          (cell->time_steps).TSTART = &(new->value);
        }
        else if (!strcmp(new->spatial_label,"tend"))
        {
          (cell->time_steps).TEND = &(new->value);
        }
        else if (!strcmp(new->spatial_label,"tabt"))
        {
          (cell->time_steps).TABT = &(new->value);
        }
        /* now add the new parameter to the list */
        if (cell->parameters != (struct Cell_parameter *)NULL)
        {
          /* add to the end of the list or overwrite if parameter already
             exists */
          current = cell->parameters;
          if (!strcmp(current->spatial_label,new->spatial_label))
          {
            /* the new parameter is the same as the first parameter, so
               replace it */
            found = 1;
            new->number_of_components = current->number_of_components;
            /*if (ALLOCATE(new->components,enum Cell_components,
              new->number_of_components))*/
            if (ALLOCATE(new->components,char *,new->number_of_components))
            {
              for (i=0;i<new->number_of_components;i++)
              {
                new->components[i] = current->components[i];
              }
            }
            else
            {
              display_message(WARNING_MESSAGE,"set_parameter_information. "
                "Unable to allocate memory for the new parameter's components");
            }
            new->next = current->next;
            cell->parameters = new;
            DEALLOCATE(current);
          }
          else
          {
            found = 0;
          }
          while (!found && (current->next != (struct Cell_parameter *)NULL))
          {
            temp = current;
            current = current->next;
            if (!strcmp(current->spatial_label,new->spatial_label))
            {
              /* the new parameter is the same as the current parameter, so
                 replace it */
              found = 1;
              new->number_of_components = current->number_of_components;
              /*if (ALLOCATE(new->components,enum Cell_components,
                new->number_of_components))*/
              if (ALLOCATE(new->components,char *,new->number_of_components))
              {
                for (i=0;i<new->number_of_components;i++)
                {
                  new->components[i] = current->components[i];
                }
              }
              else
              {
                display_message(WARNING_MESSAGE,"set_parameter_information. "
                  "Unable to allocate memory for the new parameter's "
                  "components");
              }
              new->next = current->next;
              temp->next = new;
              DEALLOCATE(current);
            }
            else
            {
              found = 0;
            }
          } /* while ... */
          if (!found)
          {
            /* new parameter not found in list, so add it to the end of
               the list */
            if (default_value)
            {
              current->next = new;
            }
            else
            {
              display_message(WARNING_MESSAGE,"set_parameter_information. "
                "Parameter ignored (%s)",new->spatial_label);
              DEALLOCATE(new);
            }
          }
        }
        else
        {
          /* create the parameters list */
          if (default_value)
          {
            cell->parameters = new;
          }
          else
          {
            display_message(WARNING_MESSAGE,"set_parameter_information. "
              "Parameter ignored (%s)",new->spatial_label);
            DEALLOCATE(new);
          }
        }
        return_code = 1;
      }
      else
      {
        display_message(ERROR_MESSAGE,"set_parameter_information. "
          "Unable to allocate memory for the parameter (**%s**) information",
          label);
        DEALLOCATE(new);
        return_code = 0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"set_parameter_information. "
        "Unable to allocate memory for the parameter **%s**",label);
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"set_parameter_information. "
      "Missing Cell window");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END set_parameter_information() */

int set_parameter_cell_component(struct Cell_window *cell,char *component)
/*******************************************************************************
LAST MODIFIED : 27 February 1999

DESCRIPTION :
Sets <component> as a cell component of the last Cell_parameter in the
cell->parameters array.
==============================================================================*/
{
  int return_code = 0;
  struct Cell_parameter *current = (struct Cell_parameter *)NULL;

  ENTER(set_parameter_cell_component);
  if (cell && component)
  {
    if (cell->parameters != (struct Cell_parameter *)NULL)
    {
      /* add the component to the last parameter in the list */
      current = cell->parameters;
      while (current->next != (struct Cell_parameter *)NULL)
      {
        current = current->next;
      }
      if (current->components == (char **)NULL)
      {
        current->number_of_components = 1;
        if (!ALLOCATE(current->components,char *,1))
        {
          current->components = (char **)NULL;
          current->number_of_components = 0;
        }
      }
      else
      {
        current->number_of_components++;
        if (!REALLOCATE(current->components,current->components,
          char *,current->number_of_components))
        {
          current->components = (char **)NULL;
          current->number_of_components = 0;
        }
      }
      if (current->components != (char **)NULL)
      {
        if (ALLOCATE(current->components[current->number_of_components-1],
          char,strlen(component)+1))
        {
					strcpy(current->components[current->number_of_components-1],
						component);
          return_code = 1;
        }
        else
        {
          display_message(ERROR_MESSAGE,"set_parameter_cell_component. "
            "Unable to allocate memory for the new component");
          return_code = 0;
          DEALLOCATE(current->components);
          current->components = (char **)NULL;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"set_parameter_cell_component. "
          "Unable to allocate memory for the new component");
        return_code = 0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"set_parameter_cell_component. "
        "No parameters defined");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"set_parameter_cell_component. "
      "Invalid arguments");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END set_parameter_cell_component() */

void destroy_cell_parameters(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 9 November 1999

DESCRIPTION :
Destroys the current list of parameters.
==============================================================================*/
{
  struct Cell_parameter *current,*tmp;
  ENTER(destroy_cell_parameters);
  if (cell)
  {
    if (cell->parameters != (struct Cell_parameter *)NULL)
    {
      current = cell->parameters;
      while (current->next != (struct Cell_parameter *)NULL)
      {
        destroy_cell_control_curve(current->control_curve[0]);
        destroy_cell_control_curve(current->control_curve[1]);
        destroy_cell_control_curve(current->control_curve[2]);
        tmp = current->next;
        DEALLOCATE(current);
        current = tmp;
      }
    }
    cell->parameters = (struct Cell_parameter *)NULL;
  }
  else
  {
    display_message(ERROR_MESSAGE,"destroy_cell_parameters. "
      "Missing Cell window");
  }
  LEAVE;
} /* END destroy_cell_parameters() */
