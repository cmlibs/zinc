/*******************************************************************************
FILE : cell_variable.c

LAST MODIFIED : 28 August 2000

DESCRIPTION :
Functions and structures for using the Cell_variable structure.
==============================================================================*/
#include <stdio.h>
#include <string.h>
#if defined (MOTIF)
#include <Xm/Xm.h>
#include <Xm/Label.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include <Xm/TextF.h>
#include <Xm/Form.h>
#endif /* if defined (MOTIF) */
#include "user_interface/user_interface.h"
#include "general/debug.h"
#include "cell/cell_variable.h"
#include "cell/cell_window.h"
#include "cell/input.h"
#include "cell/cell_control_curve.h"

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
static void spatial_toggle_changed(Widget widget,XtPointer cell_variable,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 13 February 1999

DESCRIPTION :
Callback for when the spatial toggle's state is changed.
==============================================================================*/
{
  struct Cell_variable *variable = (struct Cell_variable *)NULL;
  Boolean state;
  
  ENTER(spatial_toggle_changed);
  USE_PARAMETER(call_data);
  if (variable = (struct Cell_variable *)cell_variable)
  {
    XtVaGetValues(widget,
      XmNset,&state,
      NULL);
    if (state)
    {
      variable->spatial_switch = 1;
    }
    else
    {
      variable->spatial_switch = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"spatial_toggle_changed. "
      "Missing Cell variable");
  }
  LEAVE;
} /* END spatial_toggle_changed() */

static void control_curve_toggle_changed(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 March 1999

DESCRIPTION :
Callback for when the time variable toggle's state is changed in the variables
dialog.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  struct Cell_variable *variable = (struct Cell_variable *)NULL;
  Boolean state;
  XtPointer cell_variable;
  
  ENTER(control_curve_toggle_changed);
  USE_PARAMETER(call_data);
  USE_PARAMETER(cell);
  if (cell = (struct Cell_window *)cell_window)
  {
    XtVaGetValues(widget,
      XmNset,&state,
      XmNuserData,&cell_variable,
      NULL);
    if (variable = (struct Cell_variable *)cell_variable)
    {
      if (state)
      {
        variable->control_curve_switch = 1;
      }
      else
      {
        variable->control_curve_switch = 0;
      }
      XtVaSetValues(variable->value_widget,
        XmNsensitive,!variable->control_curve_switch,
        NULL);
    }
    else
    {
      display_message(ERROR_MESSAGE,"control_curve_toggle_changed. "
        "Missing cell variable");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"control_curve_toggle_changed. "
      "Missing Cell window");
  }
  LEAVE;
} /* END control_curve_toggle_changed() */

static void control_curve_button_callback(Widget widget,
  XtPointer cell_variable,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 9 November 1999

DESCRIPTION :
Callback for when the time variable button is pushed in the variables dialog.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  struct Cell_variable *variable = (struct Cell_variable *)NULL;
  XtPointer cell_window;
  
  ENTER(control_curve_button_callback);
  USE_PARAMETER(call_data);
  if (variable = (struct Cell_variable *)cell_variable)
  {
    if (variable->control_curve_switch)
    {
      XtVaGetValues(widget,
        XmNuserData,&cell_window,
        NULL);
      if (cell = (struct Cell_window *)cell_window)
      {
        if (!bring_up_variable_control_curve_dialog(cell,variable))
        {
          display_message(ERROR_MESSAGE,"control_curve_button_callback. "
            "Unable to bring up time variable dialog");
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"control_curve_button_callback. "
          "Missing Cell window");
      }
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"control_curve_button_callback. "
      "Missing Cell variable");
  }
  LEAVE;
} /* END control_curve_button_callback() */

static int create_variable_widgets(Widget parent,struct Cell_variable *variable,
  struct Variables_dialog_user_settings *user_settings,struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 25 March 1999

DESCRIPTION :
Add the variables widgets to the variables dialog.
==============================================================================*/
{
  int return_code = 0,small_widget_spacing = 1,large_widget_spacing = 5;
  Widget form;
  char buf[20];
  
  ENTER(create_variable_widgets);
  if (parent && variable)
  {
    form = XtVaCreateManagedWidget("cell_variable",
      xmFormWidgetClass,parent,
      NULL);
    /* create the spatial toggle */
    variable->spatial_switch_widget =
      XtVaCreateManagedWidget("cell_variable_toggle",
        xmToggleButtonWidgetClass,form,
        XmNleftOffset,small_widget_spacing,
        XmNtopOffset,small_widget_spacing,
        XmNbottomOffset,small_widget_spacing,
        XmNlabelString,XmStringCreateSimple(variable->spatial_label),
        XmNalignment,XmALIGNMENT_BEGINNING,
        XmNset,variable->spatial_switch,
        XmNleftAttachment,XmATTACH_FORM,
        XmNtopAttachment,XmATTACH_FORM,
        XmNbottomAttachment,XmATTACH_FORM,
        XmNforeground,user_settings->name_colour,
        NULL);
    /* add the value changed callback for the time variable toggle */
    XtAddCallback(variable->spatial_switch_widget,XmNvalueChangedCallback,
      spatial_toggle_changed,(XtPointer)variable);
    /* create the toggle to switch to a time variable for the variable */
    variable->control_curve_toggle =
      XtVaCreateManagedWidget("cell_variable_toggle",
        xmToggleButtonWidgetClass,form,
        XmNleftOffset,large_widget_spacing,
        XmNtopOffset,small_widget_spacing,
        XmNbottomOffset,small_widget_spacing,
        XmNlabelString,XmStringCreateSimple(" "),
        XmNalignment,XmALIGNMENT_BEGINNING,
        XmNset,variable->control_curve_switch,
        XmNsensitive,variable->control_curve_allowed,
        XmNleftAttachment,XmATTACH_WIDGET,
        XmNleftWidget,variable->spatial_switch_widget,
        XmNtopAttachment,XmATTACH_FORM,
        XmNbottomAttachment,XmATTACH_FORM,
        XmNforeground,user_settings->value_colour,
        XmNuserData,(XtPointer)variable,
        NULL);
    /* add the value changed callback for the time variable toggle */
    XtAddCallback(variable->control_curve_toggle,
      XmNvalueChangedCallback,control_curve_toggle_changed,
      (XtPointer)cell);
    /* add the push button for the time variable */
    variable->control_curve_button =
      XtVaCreateManagedWidget("cell_variable_button",
        xmPushButtonWidgetClass,form,
        XmNleftOffset,0,
        XmNtopOffset,small_widget_spacing,
        XmNbottomOffset,small_widget_spacing,
        XmNlabelString,XmStringCreateSimple(variable->control_curve_label),
        XmNalignment,XmALIGNMENT_BEGINNING,
        XmNsensitive,variable->control_curve_allowed,
        XmNleftAttachment,XmATTACH_WIDGET,
        XmNleftWidget,variable->control_curve_toggle,
        XmNtopAttachment,XmATTACH_FORM,
        XmNbottomAttachment,XmATTACH_FORM,
        XmNforeground,user_settings->value_colour,
        XmNmarginLeft,0,
        XmNshadowThickness,0,
        XmNuserData,(XtPointer)cell,
        NULL);
    /* add the callback for the time variable button */
    XtAddCallback(variable->control_curve_button,
      XmNactivateCallback,control_curve_button_callback,(XtPointer)variable);
    /* create the text field for the value of the variable */
    sprintf(buf,"%g",variable->value);
    variable->value_widget = XtVaCreateManagedWidget("cell_variable_text",
      xmTextFieldWidgetClass,form,
      XmNleftOffset,large_widget_spacing,
      XmNtopOffset,small_widget_spacing,
      XmNbottomOffset,small_widget_spacing,
      XmNvalue,buf,
      XmNeditable,True,
      XmNsensitive,True,
      XmNcolumns,15,
      XmNleftAttachment,XmATTACH_WIDGET,
      XmNleftWidget,variable->control_curve_button,
      XmNtopAttachment,XmATTACH_FORM,
      XmNbottomAttachment,XmATTACH_FORM,
      XmNforeground,user_settings->value_colour,
      NULL);
    /* add the callback to check the entries into the text field */
    XtAddCallback(variable->value_widget,XmNmodifyVerifyCallback,
      verify_text_field_modification,(XtPointer)NULL);
    /* the units label widget */
    variable->units_widget = XtVaCreateManagedWidget("cell_variable_units",
      xmLabelWidgetClass,form,
      XmNleftOffset,large_widget_spacing,
      XmNtopOffset,small_widget_spacing,
      XmNbottomOffset,small_widget_spacing,
      XmNlabelString,XmStringCreateSimple(variable->units),
      XmNalignment,XmALIGNMENT_BEGINNING,
      XmNleftAttachment,XmATTACH_WIDGET,
      XmNleftWidget,variable->value_widget,
      XmNtopAttachment,XmATTACH_FORM,
      XmNbottomAttachment,XmATTACH_FORM,
      XmNforeground,user_settings->units_colour,
      NULL);
    /* the label's label widget */
    variable->label_widget = XtVaCreateManagedWidget("cell_variable_label",
      xmLabelWidgetClass,form,
      XmNleftOffset,large_widget_spacing,
      XmNtopOffset,small_widget_spacing,
      XmNbottomOffset,small_widget_spacing,
      XmNlabelString,XmStringCreateSimple(variable->label),
      XmNalignment,XmALIGNMENT_BEGINNING,
      XmNleftAttachment,XmATTACH_WIDGET,
      XmNleftWidget,variable->units_widget,
      XmNtopAttachment,XmATTACH_FORM,
      XmNbottomAttachment,XmATTACH_FORM,
      XmNforeground,user_settings->label_colour,
      NULL);
    return_code = 1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"create_variable_widgets. "
      "Invalid arguments");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END create_variable_widgets() */


/*
Global functions
================
*/
int set_variable_information(struct Cell_window *cell,char *array,
  char *position,char *name,char *label,char *units,char *spatial,
  char *control_curve,char *value,int default_value)
/*******************************************************************************
LAST MODIFIED : 28 August 2000

DESCRIPTION :
Sets the information in the variables structure for use when creating the
variables dialog. If <default_value> is 0, then the variable is not a default
value, and MUST replace one in the list, if it does not correspond to a
variable already in then list, then it is ignored.

<array> specifies the computational array that the variable belongs in, with
a specified <position>.

?? Assume that the default values are always read in first, which sets all
variable values and their order, so that when another set of variables is
read in from a model file the existing variables will simply be replaced by
the newer ones, keeping the order the same. ??
==============================================================================*/
{
  int return_code = 0;
  struct Cell_variable *new = (struct Cell_variable *)NULL;
  struct Cell_variable *current = (struct Cell_variable *)NULL;
  struct Cell_variable *temp = (struct Cell_variable *)NULL;
  int found;

  ENTER(set_variable_information);
  if (cell)
  {
    if (ALLOCATE(new,struct Cell_variable,1))
    {
      /* initialise */
      new->label = (char *)NULL;
      new->label_widget = (Widget)NULL;
      new->units = (char *)NULL;
      new->units_widget = (Widget)NULL;
      new->spatial_switch = 0;
      new->array = ARRAY_UNKNOWN;
      new->position = 0;
      new->spatial_label = (char *)NULL;
      new->spatial_switch_widget = (Widget)NULL;
      new->value = 0.0;
      new->value_widget = (Widget)NULL;
      new->control_curve_switch = 0;
      new->control_curve_allowed = 0;
      new->control_curve_toggle = (Widget)NULL;
      new->control_curve_button = (Widget)NULL;
      new->control_curve = (struct Control_curve *)NULL;
      new->next = (struct Cell_variable *)NULL;
      if ((ALLOCATE(new->label,char,strlen(label)+1)) &&
        (ALLOCATE(new->units,char,strlen(units)+1)) &&
        (ALLOCATE(new->spatial_label,char,strlen(name)+1)) &&
        (ALLOCATE(new->control_curve_label,char,strlen("TV")+1)))
      {
				strcpy(new->label,label);
				strcpy(new->units,units);
				strcpy(new->spatial_label,name);
        if (!strncmp(array,"state",strlen("state")))
        {
          new->array = ARRAY_STATE;
        }
        else if (!strncmp(array,"parameters",strlen("parameters")))
        {
          new->array = ARRAY_PARAMETERS;
        }
        else if (!strncmp(array,"protocol",strlen("protocol")))
        {
          new->array = ARRAY_PROTOCOL;
        }
        else
        {
          new->array = ARRAY_UNKNOWN;
        }
        sscanf(position,"%d",&(new->position));
				strcpy(new->control_curve_label,"TV");
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
        /* now add the new variable to the list */
        if (cell->variables != (struct Cell_variable *)NULL)
        {
          /* add to the end of the list or overwrite if variable already
             exists */
          current = cell->variables;
          if (!strcmp(current->spatial_label,new->spatial_label))
          {
            /* the new variable is the same as the first variable, so
               replace it */
            found = 1;
            new->next = current->next;
            cell->variables = new;
            DEALLOCATE(current);
          }
          else
          {
            found = 0;
          }
          while (!found && (current->next != (struct Cell_variable *)NULL))
          {
            temp = current;
            current = current->next;
            if (!strcmp(current->spatial_label,new->spatial_label))
            {
              /* the new variable is the same as the current variable, so
                 replace it */
              found = 1;
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
            /* new variable not found, so add it to the end of the list */
            if (default_value)
            {
              current->next = new;
            }
            else
            {
              display_message(WARNING_MESSAGE,"set_variable_information. "
                "Variable ignored (%s)",new->spatial_label);
              DEALLOCATE(new);
            }
          }
        }
        else
        {
          /* create the variables list */
          if (default_value)
          {
            cell->variables = new;
          }
          else
          {
            display_message(WARNING_MESSAGE,"set_variable_information. "
              "Variable ignored (%s)",new->spatial_label);
            DEALLOCATE(new);
          }
        }
        return_code = 1;
      }
      else
      {
        display_message(ERROR_MESSAGE,"set_variable_information. "
          "Unable to allocate memory for the variable **%s**",label);
        DEALLOCATE(new);
        return_code = 0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"set_variable_information. "
        "Unable to allocate memory for the variable **%s**",label);
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"set_variable_information. "
      "Missing Cell window");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END set_variable_information() */

int add_variables_to_variables_dialog(struct Cell_window *cell,
  struct Variables_dialog_user_settings *user_settings)
/*******************************************************************************
LAST MODIFIED : 13 March 1999

DESCRIPTION :
Add the variables widgets to the variables dialog.
==============================================================================*/
{
  int return_code = 0;
  struct Cell_variable *current_variable = (struct Cell_variable *)NULL;

  ENTER(add_variables_to_dialog);
  if ((cell != (struct Cell_window *)NULL) && user_settings)
  {
    if ((cell->variables != (struct Cell_variable *)NULL) &&
      (cell->variables_dialog && cell->variables_dialog->rowcol))
    {
      current_variable = cell->variables;
      return_code = 1;
      while ((current_variable != (struct Cell_variable *)NULL) && return_code)
      {
        return_code = create_variable_widgets(cell->variables_dialog->rowcol,
          current_variable,user_settings,cell);
        current_variable = current_variable->next;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"add_variables_to_dialog. "
        "Missing required information");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"add_variables_to_dialog. "
      "Invalid arguments");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END add_variables_to_dialog() */

void destroy_cell_variables(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 9 November 1999

DESCRIPTION :
Destroys the current list of variables.
==============================================================================*/
{
  struct Cell_variable *current,*tmp;
  ENTER(destroy_cell_variables);
  if (cell)
  {
    if (cell->variables != (struct Cell_variable *)NULL)
    {
      current = cell->variables;
      while (current->next != (struct Cell_variable *)NULL)
      {
        destroy_cell_control_curve(current->control_curve);
        tmp = current->next;
        DEALLOCATE(current);
        current = tmp;
      }
      DEALLOCATE(current);
      cell->variables = (struct Cell_variable *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"destroy_cell_variables. "
      "Missing Cell window");
  }
  LEAVE;
} /* END destroy_cell_variables() */

void format_variable_widgets(struct Cell_variable *variables)
/*******************************************************************************
LAST MODIFIED : 22 February 1999

DESCRIPTION :
Formats the widget placement for all the <variables>
==============================================================================*/
{
  struct Cell_variable *current = (struct Cell_variable *)NULL;
  Dimension width,max_spatial_width,max_tv_toggle_width,max_value_width,
    max_units_width;
  Dimension height,max_height;
  
  ENTER(format_variable_widgets);
  if (variables != (struct Cell_variable *)NULL)
  {
    max_spatial_width = 0;
    max_tv_toggle_width = 0;
    max_value_width = 0;
    max_units_width = 0;
    max_height = 0;
    current = variables;
    /* loop through all varaibles, setting the maximum widths */
    while (current != (struct Cell_variable *)NULL)
    {
      XtVaGetValues(current->spatial_switch_widget,
        XmNwidth,&width,
        NULL);
      if (width > max_spatial_width)
      {
        max_spatial_width = width;
      }
      XtVaGetValues(current->control_curve_toggle,
        XmNwidth,&width,
        NULL);
      if (width > max_tv_toggle_width)
      {
        max_tv_toggle_width = width;
      }
      /* the value widget needs to be the maximum width of either the
         value text field or the time variable button */
      XtVaGetValues(current->value_widget,
        XmNwidth,&width,
        XmNheight,&height,
        NULL);
      if (width > max_value_width)
      {
        max_value_width = width;
      }
      if (height > max_height)
      {
        max_height = height;
      }
#if 0
      XtVaGetValues(current->control_curve_button,
        XmNwidth,&width,
        XmNheight,&height,
        NULL);
      if (width > max_value_width)
      {
        max_value_width = width;
      }
      if (height > max_height)
      {
        max_height = height;
      }
#endif /* 0 */
      XtVaGetValues(current->units_widget,
        XmNwidth,&width,
        NULL);
      if (width > max_units_width)
      {
        max_units_width = width;
      }
      current = current->next;
    } /* while (current->next) */
    /* reset the current pointer */
    current = variables;
    /* set all widgets to the same width */
    while (current != (struct Cell_variable *)NULL)
    {
      XtVaSetValues(current->spatial_switch_widget,
        XmNwidth,max_spatial_width,
        NULL);
      XtVaSetValues(current->control_curve_toggle,
        XmNwidth,max_tv_toggle_width,
        NULL);
      XtVaSetValues(current->value_widget,
        XmNwidth,max_value_width,
        XmNheight,max_height,
        NULL);
#if 0
      XtVaSetValues(current->control_curve_button,
        XmNwidth,max_value_width,
        XmNheight,max_height,
        NULL);
#endif /* 0 */
      XtVaSetValues(current->units_widget,
        XmNwidth,max_units_width,
        NULL);
      current = current->next;
    } /* while (current->next) */
  }
  LEAVE;
} /* END format_variable_widgets() */

void update_variable_values(struct Cell_variable *variables)
/*******************************************************************************
LAST MODIFIED : 13 February 1999

DESCRIPTION :
Updates each of the <variables> from the value text field.
==============================================================================*/
{
  struct Cell_variable *current = (struct Cell_variable *)NULL;
  float value;
  
  ENTER(update_variable_values);
  if (variables != (struct Cell_variable *)NULL)
  {
    current = variables;
    while (current != (struct Cell_variable *)NULL)
    {
      if (extract_float_from_text_field(current->value_widget,&value))
      {
        current->value = value;
      }
      current = current->next;
    }
  }
  LEAVE;
} /* END update_variable_values() */

void reset_variable_values(struct Cell_variable *variables)
/*******************************************************************************
LAST MODIFIED : 28 August 2000

DESCRIPTION :
Resets each of the <variables> to their respective values.
==============================================================================*/
{
  struct Cell_variable *current = (struct Cell_variable *)NULL;
  char buf[20];
  
  ENTER(reset_variable_values);
  if (variables != (struct Cell_variable *)NULL)
  {
    current = variables;
    while (current != (struct Cell_variable *)NULL)
    {
      if (current->value_widget != (Widget)NULL)
      {
        sprintf(buf,"%g",current->value);
        XtVaSetValues(current->value_widget,
          XmNvalue,buf,
          NULL);
      }
      current = current->next;
    }
  }
  LEAVE;
} /* END reset_variable_values() */
