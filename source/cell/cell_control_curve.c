/*******************************************************************************
FILE : cell_control_curve.c

LAST MODIFIED : 17 November 1999

DESCRIPTION :
Functions for CELL to interact with CMGUI control_curves.
==============================================================================*/
#include <stdio.h>
#include "cell/cell_window.h"
#include "cell/cell_parameter.h"
#include "cell/cell_variable.h"
#include "cell/cell_control_curve.h"
/*
Local types
===========
*/

/*
Local variables
===============
*/

/*
Local functions
===============
*/

/*
Global functions
================
*/

int bring_up_parameter_control_curve_dialog(struct Cell_window *cell,
  struct Cell_parameter *parameter)
/*******************************************************************************
LAST MODIFIED : 15 March 1999

DESCRIPTION :
Brings up the cell control curve dialog.

To start with, just bring up the control curve editor with the appropriate
curve, but, eventually, bring up a dialog which allows the user to
read/write control curves, set from default files, etc....
==============================================================================*/
{
  int return_code = 0;
	char *name;

  ENTER(bring_up_parameter_control_curve_dialog);
  if ((cell != (struct Cell_window *)NULL) &&
    (parameter != (struct Cell_parameter *)NULL))
  {
		if (!(cell->distributed).edit)
		{
			/* single cell */
			if (parameter->control_curve[0] == (struct Control_curve *)NULL)
			{
				/* need to create the control curve */
				if (parameter->control_curve[0] =
					CREATE(Control_curve)(parameter->spatial_label,LINEAR_LAGRANGE,1))
				{
					/* ACCESS so can never be destroyed */
					ACCESS(Control_curve)(parameter->control_curve[0]);
					if (!ADD_OBJECT_TO_MANAGER(Control_curve)(
						parameter->control_curve[0],
						(cell->control_curve).control_curve_manager))
					{
						DEACCESS(Control_curve)(&(parameter->control_curve[0]));
					}
				}
				else
				{
					return_code = 0;
					display_message(ERROR_MESSAGE,
						"bring_up_parameter_control_curve_dialog. "
						"Unable to create the control curve for %s",
						parameter->spatial_label);
				}
			}
			if (parameter->control_curve[0] != (struct Control_curve *)NULL)
			{
				/* temp?? */
				Control_curve_set_edit_component_range(parameter->control_curve[0],0,
					0,100);
				Control_curve_set_parameter_grid(parameter->control_curve[0],1);
				Control_curve_set_value_grid(parameter->control_curve[0],5);
				if (!bring_up_control_curve_editor_dialog(
					&((cell->control_curve).control_curve_editor_dialog),cell->window,
					(cell->control_curve).control_curve_manager,
					parameter->control_curve[0],cell->user_interface))
				{
					return_code = 0;
					display_message(ERROR_MESSAGE,
						"bring_up_parameter_control_curve_dialog. "
						"Unable to bring control curve editor dialog");
				}
				else
				{
					return_code = 1;
				}
			}
		}
		else
		{
			/* distributed model - want three control curves */
			if (parameter->control_curve[0] == (struct Control_curve *)NULL)
			{
				/* need to create the control curve */
				if (ALLOCATE(name,char,strlen(parameter->spatial_label)+
					strlen("_xi1")+1))
				{
					sprintf(name,"%s_xi1\0",parameter->spatial_label);
				}
				if (parameter->control_curve[0] =
					CREATE(Control_curve)(name,LINEAR_LAGRANGE,1))
				{
					/* ACCESS so can never be destroyed */
					ACCESS(Control_curve)(parameter->control_curve[0]);
					if (!ADD_OBJECT_TO_MANAGER(Control_curve)(
						parameter->control_curve[0],
						(cell->control_curve).control_curve_manager))
					{
						DEACCESS(Control_curve)(&(parameter->control_curve[0]));
					}
				}
				else
				{
					return_code = 0;
					display_message(ERROR_MESSAGE,
						"bring_up_parameter_control_curve_dialog. "
						"Unable to create the control curve for %s",
						parameter->spatial_label);
				}
			}
			if (parameter->control_curve[1] == (struct Control_curve *)NULL)
			{
				/* need to create the control curve */
				if (ALLOCATE(name,char,strlen(parameter->spatial_label)+
					strlen("_xi2")+1))
				{
					sprintf(name,"%s_xi2\0",parameter->spatial_label);
				}
				if (parameter->control_curve[1] =
					CREATE(Control_curve)(name,LINEAR_LAGRANGE,1))
				{
					/* ACCESS so can never be destroyed */
					ACCESS(Control_curve)(parameter->control_curve[1]);
					if (!ADD_OBJECT_TO_MANAGER(Control_curve)(
						parameter->control_curve[1],
						(cell->control_curve).control_curve_manager))
					{
						DEACCESS(Control_curve)(&(parameter->control_curve[1]));
					}
				}
				else
				{
					return_code = 0;
					display_message(ERROR_MESSAGE,
						"bring_up_parameter_control_curve_dialog. "
						"Unable to create the control curve for %s",
						parameter->spatial_label);
				}
			}
			if (parameter->control_curve[2] == (struct Control_curve *)NULL)
			{
				/* need to create the control curve */
				if (ALLOCATE(name,char,strlen(parameter->spatial_label)+
					strlen("_xi3")+1))
				{
					sprintf(name,"%s_xi3\0",parameter->spatial_label);
				}
				if (parameter->control_curve[2] =
					CREATE(Control_curve)(name,LINEAR_LAGRANGE,1))
				{
					/* ACCESS so can never be destroyed */
					ACCESS(Control_curve)(parameter->control_curve[2]);
					if (!ADD_OBJECT_TO_MANAGER(Control_curve)(
						parameter->control_curve[2],
						(cell->control_curve).control_curve_manager))
					{
						DEACCESS(Control_curve)(&(parameter->control_curve[2]));
					}
				}
				else
				{
					return_code = 0;
					display_message(ERROR_MESSAGE,
						"bring_up_parameter_control_curve_dialog. "
						"Unable to create the control curve for %s",
						parameter->spatial_label);
				}
			}
			if ((parameter->control_curve[0] != (struct Control_curve *)NULL) &&
				(parameter->control_curve[1] != (struct Control_curve *)NULL) &&
				(parameter->control_curve[2] != (struct Control_curve *)NULL))
			{
				/* temp?? */
				Control_curve_set_edit_component_range(parameter->control_curve[0],0,
					0,100);
				Control_curve_set_parameter_grid(parameter->control_curve[0],1);
				Control_curve_set_value_grid(parameter->control_curve[0],5);
				if (!bring_up_control_curve_editor_dialog(
					&((cell->control_curve).control_curve_editor_dialog),cell->window,
					(cell->control_curve).control_curve_manager,
					parameter->control_curve[0],cell->user_interface))
				{
					return_code = 0;
					display_message(ERROR_MESSAGE,
						"bring_up_parameter_control_curve_dialog. "
						"Unable to bring control curve editor dialog");
				}
				else
				{
					return_code = 1;
				}
			}
		}
  }
  else
  {
    display_message(ERROR_MESSAGE,"bring_up_parameter_control_curve_dialog. "
      "Invalid arguments");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END bring_up_parameter_control_curve_dialog() */

int bring_up_variable_control_curve_dialog(struct Cell_window *cell,
  struct Cell_variable *variable)
/*******************************************************************************
LAST MODIFIED : 25 March 1999

DESCRIPTION :
Brings up the cell control curve dialog.

To start with, just bring up the control curve editor with the appropriate
variable, but, eventually, bring up a dialog which allows the user to
read/write control curves, set from default files, etc....
==============================================================================*/
{
  int return_code = 0;

  ENTER(bring_up_variable_control_curve_dialog);
  if ((cell != (struct Cell_window *)NULL) &&
    (variable != (struct Cell_variable *)NULL))
  {
    if (variable->control_curve == (struct Control_curve *)NULL)
    {
      /* need to create the control curve */
      if (variable->control_curve =
        CREATE(Control_curve)(variable->spatial_label,LINEAR_LAGRANGE,1))
      {
        /* ACCESS so can never be destroyed */
        ACCESS(Control_curve)(variable->control_curve);
        if (!ADD_OBJECT_TO_MANAGER(Control_curve)(
          variable->control_curve,
          (cell->control_curve).control_curve_manager))
        {
          DEACCESS(Control_curve)(&(variable->control_curve));
        }
      }
      else
      {
        return_code = 0;
        display_message(ERROR_MESSAGE,"bring_up_variable_control_curve_dialog. "
          "Unable to create the control curve for %s",variable->spatial_label);
      }
    }
    if (variable->control_curve != (struct Control_curve *)NULL)
    {
      /* temp?? */
      Control_curve_set_edit_component_range(variable->control_curve,0,0.9,1.1);
			Control_curve_set_parameter_grid(variable->control_curve,1);
			Control_curve_set_value_grid(variable->control_curve,0.01);
			if (!bring_up_control_curve_editor_dialog(
        &((cell->control_curve).control_curve_editor_dialog),cell->window,
        (cell->control_curve).control_curve_manager,
        variable->control_curve,cell->user_interface))
      {
        return_code = 0;
        display_message(ERROR_MESSAGE,"bring_up_variable_control_curve_dialog. "
          "Unable to bring control curve editor dialog");
      }
      else
      {
        return_code = 1;
      }
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"bring_up_variable_control_curve_dialog. "
      "Invalid arguments");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END bring_up_variable_control_curve_dialog() */

void destroy_cell_control_curve(struct Control_curve *variable)
/*******************************************************************************
LAST MODIFIED : 25 March 1999

DESCRIPTION :
Destroys the <variable> if it exists.
==============================================================================*/
{
  ENTER(destroy_cell_control_curve);
  if (variable != (struct Control_curve *)NULL)
  {
    DESTROY(Control_curve)(&variable);
  }
  LEAVE;
} /* END destroy_cell_control_curve() */
