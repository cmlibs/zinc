/*******************************************************************************
FILE : cell_component.c

LAST MODIFIED : 15 March 2001

DESCRIPTION :
Functions and structures for using the Cell_component structure.
==============================================================================*/
#include <string.h>
#include <stdio.h>
#if defined (MOTIF)
#include <Xm/Xm.h>
#endif /* if defined (MOTIF) */
#include "user_interface/user_interface.h"
#include "general/debug.h"
#include "cell/cell_window.h"
#include "cell/cell_component.h"
#include "cell/cell_parameter.h"
#include "cell/parameter_dialog.h"
#include "general/any_object_definition.h"
#include "graphics/scene.h"

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
static char *set_component_name(char *component)
/*******************************************************************************
LAST MODIFIED : 25 August 2000

DESCRIPTION :
Returns the component name corresponding to the <component> type.
==============================================================================*/
{
  char *name;

  ENTER(set_component_name);
	if (component)
	{
		if (ALLOCATE(name,char,strlen(component)+1))
		{
			strcpy(name,component);
		}
	}
  else
  {
    name = (char *)NULL;
  }
  LEAVE;

  return (name);
} /* END set_component_name() */

static int create_cell_component(struct Cell_window *cell,char *component)
/*******************************************************************************
LAST MODIFIED : 08 September 1999

DESCRIPTION :
Creates a new cell component and adds it to the list of cell components.
==============================================================================*/
{
  int return_code = 0,i;
  struct Cell_component *new = (struct Cell_component *)NULL;
  struct Cell_component *current = (struct Cell_component *)NULL;

  ENTER(create_cell_component);
  if (cell)
  {
    if (ALLOCATE(new,struct Cell_component,1))
    {
      new->name = set_component_name(component);
      new->cell = cell;
      new->component = new->name;
      /* always create an empty region to avoid errors when checking for
         points in region */
      new->region = XCreateRegion();
      new->number_of_parameters = 0;
      new->parameters = (struct Cell_parameter **)NULL;
      new->dialog = (struct Parameter_dialog *)NULL;
      new->graphic = (struct Cell_graphic *)NULL;
      for (i=0;i<3;i++)
      {
        new->axis1[i]=0.0;
        new->axis2[i]=0.0;
        new->axis3[i]=0.0;
        new->point[i]=0.0;
      }
      new->next = (struct Cell_component *)NULL;
      if (cell->components == (struct Cell_component *)NULL)
      {
        cell->components = new;
      }
      else
      {
        current = cell->components;
        while (current->next != (struct Cell_component *)NULL)
        {
          current = current->next;
        }
        current->next = new;
      }
      return_code = 1;
    }
    else
    {
      display_message(ERROR_MESSAGE,"create_cell_component. "
        "Unable to allocate memory for the new component");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"create_cell_component. "
      "Missing Cell window");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END create_cell_component() */

static int add_parameter_to_component(struct Cell_window *cell,
  struct Cell_parameter *parameter,char *component_type)
/*******************************************************************************
LAST MODIFIED : 08 September 1999

DESCRIPTION :
Adds the <parameter> to the cell component which corresponds to the
<component_type>. If the cell component does not exist, it is created.
==============================================================================*/
{
  int return_code = 0;
  struct Cell_component *current = (struct Cell_component *)NULL;
  struct Cell_component *component = (struct Cell_component *)NULL;

  ENTER(add_parameter_to_component);
  if ((cell != (struct Cell_window *)NULL) &&
    (parameter != (struct Cell_parameter *)NULL))
  {
    current = cell->components;
    while ((current != (struct Cell_component *)NULL) &&
      (component == (struct Cell_component *)NULL))
    {
      if (!strcmp(current->component,component_type))
      {
        component = current;
      }
      current = current->next;
    }
    if (component == (struct Cell_component *)NULL)
    {
      /* need to create the component and add to the list of components */
      if (create_cell_component(cell,component_type))
      {
        /* now get the component which was added to the end of the list */
        current = cell->components;
        while (current->next != (struct Cell_component *)NULL)
        {
          current = current->next;
        }
        component = current;
      }
      else
      {
        component = (struct Cell_component *)NULL;
      }
    }
    if (component != (struct Cell_component *)NULL)
    {
      /* add the parameter to the component */
      if (component->parameters == (struct Cell_parameter **)NULL)
      {
        component->number_of_parameters = 1;
        if (!ALLOCATE(component->parameters,struct Cell_parameter *,1))
        {
          component->parameters = (struct Cell_parameter **)NULL;
          component->number_of_parameters = 0;
        }
      }
      else
      {
        component->number_of_parameters++;
        if (!REALLOCATE(component->parameters,component->parameters,
          struct Cell_parameter *,component->number_of_parameters))
        {
          component->parameters = (struct Cell_parameter **)NULL;
          component->number_of_parameters = 0;
        }
      }
      if (component->parameters != (struct Cell_parameter **)NULL)
      {
        component->parameters[component->number_of_parameters-1] = parameter;
        return_code = 1;
      }
      else
      {
        display_message(ERROR_MESSAGE,"add_parameter_to_component. "
          "Unable alolocate memory for the parameter");
        return_code = 0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"add_parameter_to_component. "
        "Unable to get corresponding component");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"add_parameter_to_component. "
      "Invalid arguments");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END add_parameter_to_component() */

/*
Global functions
================
*/

DEFINE_ANY_OBJECT(Cell_component)

enum Cell_components get_cell_component_from_string(char *component_string)
/*******************************************************************************
LAST MODIFIED : 18 February 1999

DESCRIPTION :
Returns the Cell_components corresponding to the <component>.
==============================================================================*/
{
  enum Cell_components component;
  char INA_string[] = {"INa"};
  char ICAL_string[] = {"ICaL"};
  char IPCA_string[] = {"IpCa"};
  char PASSIVE_ELEMENTS_string[] = {"Passive elements"};
  char ICAB_string[] = {"ICab"};
  char ICAT_string[] = {"ICaT"};
  char EXTRACELLULAR_CA_string[] = {"Extracellular Ca"};
  char INACA_string[] = {"INaCa"};
  char INSCA_string[] = {"InsCa"};
  char MEMBRANE_string[] = {"Membrane"};
  char JSRCA_string[] = {"JSR Ca"};
  char IREL_string[] = {"Irel"};
  char ITR_string[] = {"Itr"};
  char IUP_string[] = {"Iup"};
  char NSRCA_string[] = {"NSR Ca"};
  char ILEAK_string[] = {"Ileak"};
  char MITOCHONDRIA_string[] = {"Mitochondria"};
  char CALMODULIN_string[] = {"Calmodulin"};
  char INTRACELLULAR_CA_string[] = {"Intracellular Ca"};
  char TROPONIN_string[] = {"Troponin"};
  char TROPOMYOSIN_string[] = {"Tropomyosin"};
  char STEADY_STATE_MECHANICS_string[] = {"Steady state mechanics"};
  char CROSSBRIDGES_string[] = {"Crossbridges"};
  char ITO_string[] = {"Ito"};
  char IKP_string[] = {"IKp"};
  char IK1_string[] = {"IK1"};
  char IKS_string[] = {"IKs"};
  char IKR_string[] = {"IKr"};
  char INAK_string[] = {"INaK"};
  char INTRACELLULAR_MG_string[] = {"Intracellular Mg"};
  char INTRACELLULAR_NA_string[] = {"Intracellular Na"};
  char INTRACELLULAR_K_string[] = {"Intracellular K"};
  char EXTRACELLULAR_NA_string[] = {"Extracellular Na"};
  char EXTRACELLULAR_K_string[] = {"Extracellular K"};
  char INAB_string[] = {"INab"};
  char IF_string[] = {"If"};
  char PARAMETERS_string[] = {"Parameters"};

  ENTER(get_cell_component_from_string);
  if (!strcmp(component_string,INA_string))
  {
    component = INA;
  }
  else if (!strcmp(component_string,ICAL_string))
  {
    component = ICAL;
  }
  else if (!strcmp(component_string,IPCA_string))
  {
    component = IPCA;
  }
  else if (!strcmp(component_string,PASSIVE_ELEMENTS_string))
  {
    component = PASSIVE_ELEMENTS;
  }
  else if (!strcmp(component_string,ICAB_string))
  {
    component = ICAB;
  }
  else if (!strcmp(component_string,ICAT_string))
  {
    component = ICAT;
  }
  else if (!strcmp(component_string,EXTRACELLULAR_CA_string))
  {
    component = EXTRACELLULAR_CA;
  }
  else if (!strcmp(component_string,INACA_string))
  {
    component = INACA;
  }
  else if (!strcmp(component_string,INSCA_string))
  {
    component = INSCA;
  }
  else if (!strcmp(component_string,MEMBRANE_string))
  {
    component = MEMBRANE;
  }
  else if (!strcmp(component_string,JSRCA_string))
  {
    component = JSRCA;
  }
  else if (!strcmp(component_string,IREL_string))
  {
    component = IREL;
  }
  else if (!strcmp(component_string,ITR_string))
  {
    component = ITR;
  }
  else if (!strcmp(component_string,IUP_string))
  {
    component = IUP;
  }
  else if (!strcmp(component_string,NSRCA_string))
  {
    component = NSRCA;
  }
  else if (!strcmp(component_string,ILEAK_string))
  {
    component = ILEAK;
  }
  else if (!strcmp(component_string,MITOCHONDRIA_string))
  {
    component = MITOCHONDRIA;
  }
  else if (!strcmp(component_string,CALMODULIN_string))
  {
    component = CALMODULIN;
  }
  else if (!strcmp(component_string,INTRACELLULAR_CA_string))
  {
    component = INTRACELLULAR_CA;
  }
  else if (!strcmp(component_string,TROPONIN_string))
  {
    component = TROPONIN;
  }
  else if (!strcmp(component_string,TROPOMYOSIN_string))
  {
    component = TROPOMYOSIN;
  }
  else if (!strcmp(component_string,STEADY_STATE_MECHANICS_string))
  {
    component = STEADY_STATE_MECHANICS;
  }
  else if (!strcmp(component_string,CROSSBRIDGES_string))
  {
    component = CROSSBRIDGES;
  }
  else if (!strcmp(component_string,ITO_string))
  {
    component = ITO;
  }
  else if (!strcmp(component_string,IKP_string))
  {
    component = IKP;
  }
  else if (!strcmp(component_string,IK1_string))
  {
    component = IK1;
  }
  else if (!strcmp(component_string,IKS_string))
  {
    component = IKS;
  }
  else if (!strcmp(component_string,IKR_string))
  {
    component = IKR;
  }
  else if (!strcmp(component_string,INAK_string))
  {
    component = INAK;
  }
  else if (!strcmp(component_string,INTRACELLULAR_MG_string))
  {
    component = INTRACELLULAR_MG;
  }
  else if (!strcmp(component_string,INTRACELLULAR_NA_string))
  {
    component = INTRACELLULAR_NA;
  }
  else if (!strcmp(component_string,INTRACELLULAR_K_string))
  {
    component = INTRACELLULAR_K;
  }
  else if (!strcmp(component_string,EXTRACELLULAR_NA_string))
  {
    component = EXTRACELLULAR_NA;
  }
  else if (!strcmp(component_string,EXTRACELLULAR_K_string))
  {
    component = EXTRACELLULAR_K;
  }
  else if (!strcmp(component_string,INAB_string))
  {
    component = INAB;
  }
  else if (!strcmp(component_string,IF_string))
  {
    component = IF;
  }
  else if (!strcmp(component_string,PARAMETERS_string))
  {
    component = PARAMETERS;
  }
  else
  {
    component = UNKNOWN_COMPONENT;
    display_message(WARNING_MESSAGE,"get_component_from_string. "
      "Unknown component: %s",component_string);
  }
  LEAVE;
  return(component);
} /* END get_cell_component_from_string() */

void destroy_cell_component_list(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 15 March 2001

DESCRIPTION :
Deallocates the memory associated with the component list.
==============================================================================*/
{
  struct Cell_component *current = (struct Cell_component *)NULL;
  struct Cell_component *tmp = (struct Cell_component *)NULL;

  ENTER(destroy_cell_component_list);
  if (cell)
  {
    if (cell->components != (struct Cell_component *)NULL)
    {
      current = cell->components;
      while (current->next != (struct Cell_component *)NULL)
      {
        /* remove the graphic from the scene */
        if ((current->graphic != (struct Cell_graphic *)NULL) &&
          (current->graphic->graphics_object != (struct GT_object *)NULL))
        {
          Scene_remove_graphics_object((cell->cell_3d).scene,
            current->graphic->graphics_object);
        }
        close_parameter_dialog(current);
        tmp = current->next;
        DEALLOCATE(current);
        current = tmp;
      }
      if ((current->graphic != (struct Cell_graphic *)NULL) &&
        (current->graphic->graphics_object != (struct GT_object *)NULL))
      {
        Scene_remove_graphics_object((cell->cell_3d).scene,
          current->graphic->graphics_object);
      }
      close_parameter_dialog(current);
      destroy_cell_graphic(current);
      DEALLOCATE(current);
      cell->components = (struct Cell_component *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"destroy_cell_component_list. "
      "Missing cell window");
  }
  LEAVE;
} /* END destroy_cell_component_list() */

int create_cell_component_list(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 08 September 1999

DESCRIPTION :
Creates the cell component list from the defined parameters.
==============================================================================*/
{
  int return_code = 0,i;
  struct Cell_parameter *current = (struct Cell_parameter *)NULL;

  ENTER(create_cell_component_list);
  if (cell)
  {
    if (cell->components != (struct Cell_component *)NULL)
    {
      destroy_cell_component_list(cell);
    }
    current = cell->parameters;
    return_code = 1;
    while ((current != (struct Cell_parameter *)NULL) && return_code)
    {
      for (i=0;(i<current->number_of_components)&&return_code;i++)
      {
        /* add a pointer to the current parameter to all of its components */
        return_code = add_parameter_to_component(cell,current,
          current->components[i]);
      }
      current = current->next;
    } /* while (current != (struct Cell_parameter *)NULL) */
  }
  else
  {
    display_message(ERROR_MESSAGE,"create_cell_component_list. "
      "Invalid arguments");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END create_cell_component_list() */
