/*******************************************************************************
FILE : parameter_dialog.c

LAST MODIFIED : 29 November 1999

DESCRIPTION :
Functions and structures for using the parameter dialog boxes.
==============================================================================*/
#include <stdio.h>
#if defined (MOTIF)
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Xm/MwmUtil.h>
#include <Xm/DialogS.h>
#include <Xm/Label.h>
#include <Xm/Protocols.h>
#include <Xm/Form.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include <Xm/TextF.h>
#endif /* if defined (MOTIF) */
#include "user_interface/user_interface.h"
#include "general/debug.h"
#include "cell/parameter_dialog.h"
#include "cell/parameter_dialog.uid64"
#include "cell/cell_component.h"
#include "cell/cell_window.h"
#include "cell/input.h"
#include "cell/cell_control_curve.h"

/*
Module types
============
*/
typedef struct
/*******************************************************************************
LAST MODIFIED : 13 March 1999

DESCRIPTION :
Holds the information related to the parameter dialog specified in the resource
file.
==============================================================================*/
{
	Pixel name_colour,value_colour,units_colour,label_colour;
} User_settings;

/*
Module variables
================
*/
#if defined (MOTIF)
static int parameter_dialog_hierarchy_open=0;
static MrmHierarchy parameter_dialog_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
================
*/
static void identify_parameter_rowcol(Widget widget,
  XtPointer cell_component,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 February 1999

DESCRIPTION :
ID's the parameter row column manager widget
==============================================================================*/
{
  struct Cell_component *component = (struct Cell_component *)NULL;
  
  ENTER(idetify_parameter_rowcol);
  USE_PARAMETER(call_data);
  if (component = (struct Cell_component *)cell_component)
  {
    if (component->dialog != (struct Parameter_dialog *)NULL)
    {
      component->dialog->parameter_rowcol = widget;
    }
    else
    {
      display_message(ERROR_MESSAGE,"identify_parameter_rowcol. "
        "Missing parameter dialog");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"identify_parameter_rowcol. "
      "Missing component");
  }
  LEAVE;
} /* END identify_component_rowcol() */

#if defined (OLD_CODE)
static void update_value(Widget widget,char *name,float *value,float new_value)
/*******************************************************************************
LAST MODIFIED : 13 March 1999

DESCRIPTION :
The the XmNlabelString attribute of the <widget> is the <name>, then <value>
is set to the <new_value>.
==============================================================================*/
{
  char *name_string;
  XmString str;
  
  ENTER(update_value);
  if (widget && name)
  {
    XtVaGetValues(widget,
      XmNlabelString,&str,
      NULL);
    XmStringGetLtoR(str,XmSTRING_DEFAULT_CHARSET,&name_string);
    if (strcmp(name_string,name) == 0)
    {
      *value = new_value;
    }
  }
  LEAVE;
} /* END update_value() */
#endif /* defined (OLD_CODE) */

static void update_parameter_values_from_dialog(
  struct Cell_component *component)
/*******************************************************************************
LAST MODIFIED : 13 March 1999

DESCRIPTION :
Updates each of the <component>'s parameters. Checks for time step parameters,
and updates if requires.
==============================================================================*/
{
  float value;
  int i;
  
  ENTER(update_parameter_values_from_dialog);
  if (component != (struct Cell_component *)NULL)
  {
#if 0
    time_step_values[0] = &((component->cell->time_steps).TEND);
    time_step_values[1] = &((component->cell->time_steps).TSTART);
    time_step_values[2] = &((component->cell->time_steps).TABT);
#endif
    for (i=0;i<component->number_of_parameters;i++)
    {
      if (extract_float_from_text_field(component->dialog->values[i],&value))
      {
        component->parameters[i]->value = value;
#if 0
        for (j=0;j<number_of_names;j++)
        {
          update_value(component->dialog->spatial_toggles[i],time_step_names[j],
            time_step_values[j],value);
        }
#endif
      }
    }
  }
  LEAVE;
} /* END update_parameter_values_from_dialog() */

static void update_dialog_from_parameter_values(
  struct Cell_component *component)
/*******************************************************************************
LAST MODIFIED : 18 February 1999

DESCRIPTION :
Resets each of the components' value widgets to their respective parameter
values
==============================================================================*/
{
  char buf[20];
  int i;
  struct Cell_component *current = (struct Cell_component *)NULL;
  Boolean state;
  
  ENTER(update_dialog_from_parameter_values);
  if ((component != (struct Cell_component *)NULL) && component->cell &&
    (component->cell->components != (struct Cell_component *)NULL))
  {
    /* loop through all components to update all values */
    current = component->cell->components;
    while (current != (struct Cell_component *)NULL)
    {
      /* if the widgets have been created, update their values */
      if ((current->dialog != (struct Parameter_dialog *)NULL) &&
        (current->dialog->values != (Widget *)NULL) &&
        (current->dialog->spatial_toggles != (Widget *)NULL))
      {
        for (i=0;i<current->number_of_parameters;i++)
        {
          sprintf(buf,"%g\0",current->parameters[i]->value);
          XtVaSetValues(current->dialog->values[i],
            XmNvalue,buf,
            NULL);
          if (current->parameters[i]->spatial_switch)
          {
            state = True;
          }
          else
          {
            state = False;
          }
          XtVaSetValues(current->dialog->spatial_toggles[i],
            XmNset,state,
            NULL);
					XtVaSetValues(current->dialog->time_variable_toggles[i],
            XmNsensitive,current->parameters[i]->time_variable_allowed,
            NULL);
        } /* for (i..) */
      } /* if widgets exist */
      current = current->next;
    }
  }
  LEAVE;
} /* END update_dialog_from_parameter_values() */

static void ok_button_callback(Widget widget,
  XtPointer cell_component,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 February 1999

DESCRIPTION :
Callback for the OK button in the parameter dialog.
==============================================================================*/
{
  struct Cell_component *component = (struct Cell_component *)NULL;
  
  ENTER(ok_button_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (component = (struct Cell_component *)cell_component)
  {
    if (component->dialog)
    {
      if (component->dialog->shell)
      {
        XtPopdown(component->dialog->shell);
        if (component->parameters)
        {
          update_parameter_values_from_dialog(component);
          update_dialog_from_parameter_values(component);
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"ok_button_callback. "
          "Missing component parameter dialog shell");
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"ok_button_callback. "
        "Missing component parameter dialog");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"ok_button_callback. "
      "Missing component");
  }
  LEAVE;
} /* END ok_button_callback() */

static void apply_button_callback(Widget widget,
  XtPointer cell_component,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 February 1999

DESCRIPTION :
Callback for the Apply button in the parameter dialog.
==============================================================================*/
{
  struct Cell_component *component = (struct Cell_component *)NULL;
  
  ENTER(apply_button_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (component = (struct Cell_component *)cell_component)
  {
    if (component->dialog)
    {
      if (component->parameters)
      {
        update_parameter_values_from_dialog(component);
        update_dialog_from_parameter_values(component);
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"apply_button_callback. "
        "Missing component parameter dialog");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"apply_button_callback. "
      "Missing component");
  }
  LEAVE;
} /* END apply_button_callback() */

static void reset_button_callback(Widget widget,
  XtPointer cell_component,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 February 1999

DESCRIPTION :
Callback for the RESET button
==============================================================================*/
{
  struct Cell_component *component = (struct Cell_component *)NULL;
  
  ENTER(reset_button_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (component = (struct Cell_component *)cell_component)
  {
    update_dialog_from_parameter_values(component);    
  }
  else
  {
    display_message(ERROR_MESSAGE,"reset_button_callback. "
      "Missing component");
  }
  LEAVE;
} /* END reset_button_callback() */

static void cancel_button_callback(Widget widget,
  XtPointer cell_component,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 February 1999

DESCRIPTION :
Callback for the CANCEL button
==============================================================================*/
{
  struct Cell_component *component = (struct Cell_component *)NULL;
  
  ENTER(cancel_button_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (component = (struct Cell_component *)cell_component)
  {
    if (component->dialog)
    {
      if (component->dialog->shell)
      {
        XtPopdown(component->dialog->shell);
        update_dialog_from_parameter_values(component);    
      }
      else
      {
        display_message(ERROR_MESSAGE,"cancel_button_callback. "
          "Missing component parameter dialog shell");
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"cancel_button_callback. "
        "Missing component parameter dialog");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"cancel_button_callback. "
      "Missing component");
  }
  LEAVE;
} /* END cancel_button_callback() */

static void help_button_callback(Widget widget,
  XtPointer cell_component,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 February 1999

DESCRIPTION :
Callback for the HELP button
==============================================================================*/
{
  struct Cell_component *component = (struct Cell_component *)NULL;
  
  ENTER(help_button_callback);
  USE_PARAMETER(component);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (component = (struct Cell_component *)cell_component)
  {
    display_message(INFORMATION_MESSAGE,"help_button_callback. "
      "sorry, no help available for the parameter dialogs!!!\n");
  }
  else
  {
    display_message(ERROR_MESSAGE,"help_button_callback. "
      "Missing component");
  }
  LEAVE;
} /* END help_button_callback() */

static void parameter_dialog_destroy_callback(Widget widget,
  XtPointer cell_component,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 February 1999

DESCRIPTION :
Callback for when the parameter dialog is destroyed via the window
manager menu ??
==============================================================================*/
{
  struct Cell_component *component = (struct Cell_component *)NULL;
  
  ENTER(parameter_dialog_destroy_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (component = (struct Cell_component *)cell_component)
  {
    if (component->dialog)
    {
      if (component->dialog->shell)
      {
        /* remove the shell from the shell list */
        destroy_Shell_list_item_from_shell(&(component->dialog->shell),
          component->cell->user_interface);
        /* make sure the dialog is no longer visible */
        XtPopdown(component->dialog->shell);
        /* Unmanage the shell */
        XtUnmanageChild(component->dialog->shell);
        /*display_message(INFORMATION_MESSAGE,
          "parameter_dialog_destroy_callback. "
          "Unmanage parameter dialog shell\n");*/
      }
      DEALLOCATE(component->dialog);
      component->dialog = (struct Parameter_dialog *)NULL;
      /*display_message(INFORMATION_MESSAGE,
        "parameter_dialog_destroy_callback. "
        "DEALLOCATE component dialog\n");*/
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"parameter_dialog_destroy_callback. "
      "Missing component");
  }
  LEAVE;
} /* END component_dialog_destroy_callback() */

static void spatial_toggle_changed(Widget widget,XtPointer cell_parameter,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 February 1999

DESCRIPTION :
Callback for when the spatial toggle's state is changed.
==============================================================================*/
{
  struct Cell_parameter *parameter = (struct Cell_parameter *)NULL;
  Boolean state;
  
  ENTER(spatial_toggle_changed);
  USE_PARAMETER(call_data);
  if (parameter = (struct Cell_parameter *)cell_parameter)
  {
    if (*(parameter->edit_distributed))
    {
      /* if editing distributed, can't change the spatial toggle */
      XtVaGetValues(widget,
        XmNset,&state,
        NULL);
      XtVaSetValues(widget,
        XmNset,!state,
        NULL);
    }
    else
    {
      XtVaGetValues(widget,
        XmNset,&state,
        NULL);
      if (state)
      {
        parameter->spatial_switch = 1;
      }
      else
      {
        parameter->spatial_switch = 0;
      }
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"spatial_toggle_changed. "
      "Missing Cell parameter");
  }
  LEAVE;
} /* END spatial_toggle_changed() */

static void time_variable_toggle_changed(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 March 1999

DESCRIPTION :
Callback for when the time variable toggle's state is changed.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  struct Cell_parameter *parameter = (struct Cell_parameter *)NULL;
  struct Cell_component *component = (struct Cell_component *)NULL;
  Boolean state;
  XtPointer cell_parameter;
  int i,j;
  XmString str;
  char *string;
  
  ENTER(time_variable_toggle_changed);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    XtVaGetValues(widget,
      XmNset,&state,
      XmNuserData,&cell_parameter,
      NULL);
    if (parameter = (struct Cell_parameter *)cell_parameter)
    {
      if (state)
      {
        parameter->time_variable_switch = 1;
      }
      else
      {
        parameter->time_variable_switch = 0;
      }
      /* loop through all the components for this parameter, updating the time
         variable widgets (based on there always being more components in
         total than a single parameter's number of components) */
      component = cell->components;
      while (component != (struct Cell_component *)NULL)
      {
        for (i=0;i<parameter->number_of_components;i++)
        {
          if (component->component == parameter->components[i])
          {
            /* the parameter appears in this component's dialog, so update the
               the widgets */
            if (component->dialog != (struct Parameter_dialog *)NULL)
            {
              for (j=0;j<component->dialog->number_of_parameters;j++)
              {
                XtVaGetValues(component->dialog->spatial_toggles[j],
                  XmNlabelString,&str,
                  NULL);
                XmStringGetLtoR(str,XmSTRING_DEFAULT_CHARSET,&string);
                if (!strcmp(string,parameter->spatial_label))
                {
									if ((component->cell->distributed).edit)
									{
										XtVaSetValues(component->dialog->values[j],
											XmNsensitive,!parameter->time_variable_switch,
											NULL);
									}
									else
									{
										XtVaSetValues(component->dialog->values[j],
											XmNsensitive,True,
											NULL);
									}
                  XtVaSetValues(component->dialog->time_variable_buttons[j],
                    XmNsensitive,parameter->time_variable_switch,
                    NULL);
                }
                XmStringFree(str);
                XtFree(string);
              } /* for (j=0..number of parameters) */
            } /* if (parameter dialog) */
          } /* if (component..) */
        } /* for (i=0...number of components) */
        component = component->next;
      } /* while (component ...) */
    }
    else
    {
      display_message(ERROR_MESSAGE,"time_variable_toggle_changed. "
        "Missing cell parameter");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"time_variable_toggle_changed. "
      "Missing Cell window");
  }
  LEAVE;
} /* END time_variable_toggle_changed() */

static void time_variable_button_callback(Widget widget,
  XtPointer cell_parameter,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 9 November 1999

DESCRIPTION :
Callback for when the time variable button is pushed.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  struct Cell_parameter *parameter = (struct Cell_parameter *)NULL;
  XtPointer cell_window;
  
  ENTER(time_variable_button_callback);
  USE_PARAMETER(call_data);
  if (parameter = (struct Cell_parameter *)cell_parameter)
  {
    if (parameter->time_variable_switch)
    {
      XtVaGetValues(widget,
        XmNuserData,&cell_window,
        NULL);
      if (cell = (struct Cell_window *)cell_window)
      {
        if (!bring_up_parameter_control_curve_dialog(cell,parameter))
        {
          display_message(ERROR_MESSAGE,"time_variable_button_callback. "
            "Unable to bring up time variable dialog");
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"time_variable_button_callback. "
          "Missing Cell window");
      }
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"time_variable_button_callback. "
      "Missing Cell parameter");
  }
  LEAVE;
} /* END time_variable_button_callback() */

static int create_parameter_widgets(Widget parent,
  struct Parameter_dialog *dialog,struct Cell_parameter *parameter,
  int parameter_position,User_settings *user_settings,
  struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 15 March 1999

DESCRIPTION :
Creates the widgets associated with the <parameter> in the <dialog> and adds
them to the dialog's list of widgets. <parameter_position> specifies the
parameter's position in the list.

NOTE - need to keep the parameter widgets separate from the Cell_parameter
       struct since each parameter can appear in more than one dialog box.
==============================================================================*/
{
  int return_code = 0,small_widget_spacing = 1,large_widget_spacing = 5;
  Widget form;
  char buf[20];

  ENTER(create_parameter_widgets);
  if (parent && dialog && parameter)
  {
    /* allocate memory for the widgets */
    if (parameter_position == 0)
    {
      if (!ALLOCATE(dialog->labels,Widget,1))
      {
        dialog->labels = (Widget *)NULL;
      }
      if (!ALLOCATE(dialog->units,Widget,1))
      {
        dialog->units = (Widget *)NULL;
      }
      if (!ALLOCATE(dialog->values,Widget,1))
      {
        dialog->values = (Widget *)NULL;
      }
      if (!ALLOCATE(dialog->time_variable_toggles,Widget,1))
      {
        dialog->time_variable_toggles = (Widget *)NULL;
      }
      if (!ALLOCATE(dialog->time_variable_buttons,Widget,1))
      {
        dialog->time_variable_buttons = (Widget *)NULL;
      }
      if (!ALLOCATE(dialog->spatial_toggles,Widget,1))
      {
        dialog->spatial_toggles = (Widget *)NULL;
      }
    }
    else
    {
      if (!REALLOCATE(dialog->labels,dialog->labels,Widget,
        parameter_position+1))
      {
        dialog->labels = (Widget *)NULL;
      }
      if (!REALLOCATE(dialog->units,dialog->units,Widget,
        parameter_position+1))
      {
        dialog->units = (Widget *)NULL;
      }
      if (!REALLOCATE(dialog->values,dialog->values,Widget,
        parameter_position+1))
      {
        dialog->values = (Widget *)NULL;
      }
      if (!REALLOCATE(dialog->time_variable_toggles,
        dialog->time_variable_toggles,Widget,
        parameter_position+1))
      {
        dialog->time_variable_toggles = (Widget *)NULL;
      }
      if (!REALLOCATE(dialog->time_variable_buttons,
        dialog->time_variable_buttons,Widget,
        parameter_position+1))
      {
        dialog->time_variable_buttons = (Widget *)NULL;
      }
      if (!REALLOCATE(dialog->spatial_toggles,dialog->spatial_toggles,Widget,
        parameter_position+1))
      {
        dialog->spatial_toggles = (Widget *)NULL;
      }
    }
    if ((dialog->labels != (Widget *)NULL) &&
      (dialog->units != (Widget *)NULL) &&
      (dialog->values != (Widget *)NULL) &&
      (dialog->time_variable_toggles != (Widget *)NULL) &&
      (dialog->time_variable_buttons != (Widget *)NULL) &&
      (dialog->spatial_toggles != (Widget *)NULL))
    {
      form = XtVaCreateManagedWidget("cell_variable",
        xmFormWidgetClass,parent,
        NULL);
      /* create the spatial toggle */
      dialog->spatial_toggles[parameter_position] =
        XtVaCreateManagedWidget("cell_parameter_toggle",
          xmToggleButtonWidgetClass,form,
          XmNleftOffset,small_widget_spacing,
          XmNtopOffset,small_widget_spacing,
          XmNbottomOffset,small_widget_spacing,
          XmNlabelString,XmStringCreateSimple(parameter->spatial_label),
          XmNalignment,XmALIGNMENT_BEGINNING,
          XmNset,parameter->spatial_switch,
          XmNleftAttachment,XmATTACH_FORM,
          XmNtopAttachment,XmATTACH_FORM,
          XmNbottomAttachment,XmATTACH_FORM,
          XmNforeground,user_settings->name_colour,
          NULL);
      /* add the value changed callback for the spatial toggle */
      XtAddCallback(dialog->spatial_toggles[parameter_position],
        XmNvalueChangedCallback,spatial_toggle_changed,(XtPointer)parameter);
      /* the time variable toggle */
      dialog->time_variable_toggles[parameter_position] =
        XtVaCreateManagedWidget("cell_parameter_toggle",
          xmToggleButtonWidgetClass,form,
          XmNleftOffset,large_widget_spacing,
          XmNtopOffset,small_widget_spacing,
          XmNbottomOffset,small_widget_spacing,
          XmNlabelString,XmStringCreateSimple(" "),
          XmNalignment,XmALIGNMENT_BEGINNING,
          XmNset,parameter->time_variable_switch,
          XmNsensitive,parameter->time_variable_allowed,
          XmNleftAttachment,XmATTACH_WIDGET,
          XmNleftWidget,dialog->spatial_toggles[parameter_position],
          XmNtopAttachment,XmATTACH_FORM,
          XmNbottomAttachment,XmATTACH_FORM,
          XmNforeground,user_settings->value_colour,
          XmNuserData,(XtPointer)parameter,
          NULL);
      /* add the value changed callback for the time variable toggle */
      XtAddCallback(dialog->time_variable_toggles[parameter_position],
        XmNvalueChangedCallback,time_variable_toggle_changed,
        (XtPointer)cell);
      /* add the push button for the time variable */
      dialog->time_variable_buttons[parameter_position] =
        XtVaCreateManagedWidget("cell_parameter_button",
          xmPushButtonWidgetClass,form,
          XmNleftOffset,0,
          XmNtopOffset,small_widget_spacing,
          XmNbottomOffset,small_widget_spacing,
          XmNlabelString,XmStringCreateSimple("TV"),
          XmNalignment,XmALIGNMENT_BEGINNING,
          XmNsensitive,parameter->time_variable_allowed,
          XmNleftAttachment,XmATTACH_WIDGET,
          XmNleftWidget,dialog->time_variable_toggles[parameter_position],
          XmNtopAttachment,XmATTACH_FORM,
          XmNbottomAttachment,XmATTACH_FORM,
          XmNforeground,user_settings->value_colour,
          XmNmarginLeft,0,
          XmNshadowThickness,0,
          XmNuserData,(XtPointer)cell,
          NULL);
      /* add the callback for the time variable button */
      XtAddCallback(dialog->time_variable_buttons[parameter_position],
        XmNactivateCallback,time_variable_button_callback,(XtPointer)parameter);
      /* create the text field for the value of the variable */
      sprintf(buf,"%g\0",parameter->value);
      dialog->values[parameter_position] =
        XtVaCreateManagedWidget("cell_parameter_text",
          xmTextFieldWidgetClass,form,
          XmNleftOffset,large_widget_spacing,
          XmNtopOffset,small_widget_spacing,
          XmNbottomOffset,small_widget_spacing,
          XmNvalue,buf,
          XmNeditable,True,
          XmNsensitive,!parameter->time_variable_switch,
          XmNcolumns,15,
          XmNleftAttachment,XmATTACH_WIDGET,
          XmNleftWidget,dialog->time_variable_buttons[parameter_position],
          XmNtopAttachment,XmATTACH_FORM,
          XmNbottomAttachment,XmATTACH_FORM,
          XmNforeground,user_settings->value_colour,
          NULL);
      /* add the callback to check the entries into the text field */
      XtAddCallback(dialog->values[parameter_position],XmNmodifyVerifyCallback,
        verify_text_field_modification,(XtPointer)NULL);
      /* the units label widget */
      dialog->units[parameter_position] =
        XtVaCreateManagedWidget("cell_parameter_units",
          xmLabelWidgetClass,form,
          XmNleftOffset,large_widget_spacing,
          XmNtopOffset,small_widget_spacing,
          XmNbottomOffset,small_widget_spacing,
          XmNlabelString,XmStringCreateSimple(parameter->units),
          XmNalignment,XmALIGNMENT_BEGINNING,
          XmNleftAttachment,XmATTACH_WIDGET,
          XmNleftWidget,dialog->values[parameter_position],
          XmNtopAttachment,XmATTACH_FORM,
          XmNbottomAttachment,XmATTACH_FORM,
          XmNforeground,user_settings->units_colour,
          NULL);
      /* the label's label widget */
      dialog->labels[parameter_position] =
        XtVaCreateManagedWidget("cell_variable_label",
          xmLabelWidgetClass,form,
          XmNleftOffset,large_widget_spacing,
          XmNtopOffset,small_widget_spacing,
          XmNbottomOffset,small_widget_spacing,
          XmNlabelString,XmStringCreateSimple(parameter->label),
          XmNalignment,XmALIGNMENT_BEGINNING,
          XmNleftAttachment,XmATTACH_WIDGET,
          XmNleftWidget,dialog->units[parameter_position],
          XmNtopAttachment,XmATTACH_FORM,
          XmNbottomAttachment,XmATTACH_FORM,
          XmNforeground,user_settings->label_colour,
          NULL);
      return_code = 1;
    }
    else
    {
      display_message(ERROR_MESSAGE,"create_parameter_widgets. "
        "Unable to allocate memory for the parameters");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"create_parameter_widgets. "
      "Invalid arguments");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END create_parameter_widgets() */

static int add_parameters_to_parameter_dialog(struct Cell_component *component,
  User_settings *user_settings)
/*******************************************************************************
LAST MODIFIED : 13 March 1999

DESCRIPTION :
Adds the parameters associated with the <component> to the component's dialog.
==============================================================================*/
{
  int return_code = 0,i;

  ENTER(add_parameters_to_parameter_dialog);
  if ((component != (struct Cell_component *)NULL) && user_settings)
  {
    if (component->dialog && component->dialog->parameter_rowcol)
    {
      return_code = 1;
      for (i=0;(i<component->number_of_parameters)&&return_code;i++)
      {
        return_code =
          create_parameter_widgets(component->dialog->parameter_rowcol,
            component->dialog,component->parameters[i],i,user_settings,
            component->cell);
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"add_parameters_to_parameter_dialog. "
        "Missing required widgets");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"add_parameters_to_parameter_dialog. "
      "Invalid arguments");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END add_parameters_to_parameter_dialog() */

void format_parameter_dialog_widgets(struct Parameter_dialog *dialog)
/*******************************************************************************
LAST MODIFIED : 19 February 1999

DESCRIPTION :
Formats the widget placement for all the parameter widgets in the <dialog>
==============================================================================*/
{
  Position width,max_spatial_width,max_value_width,max_units_width;
  int i;
  
  ENTER(format_parameter_dialog_widgets);
  if (dialog)
  {
    if ((dialog->labels != (Widget *)NULL) && (dialog->units != (Widget *)NULL)
      && (dialog->values != (Widget *)NULL) &&
      (dialog->spatial_toggles != (Widget *)NULL))
    {
      max_spatial_width = 0;
      max_value_width = 0;
      max_units_width = 0;
      /* loop through all parameters, setting the maximum widths */
      for (i=0;i<dialog->number_of_parameters;i++)
      {
        XtVaGetValues(dialog->spatial_toggles[i],
          XmNwidth,&width,
          NULL);
        if (width > max_spatial_width)
        {
          max_spatial_width = width;
        }
        XtVaGetValues(dialog->values[i],
          XmNwidth,&width,
          NULL);
        if (width > max_value_width)
        {
          max_value_width = width;
        }
        XtVaGetValues(dialog->units[i],
          XmNwidth,&width,
          NULL);
        if (width > max_units_width)
        {
          max_units_width = width;
        }
      } /* for (i..) */
      /* set all widgets to the same width */
      for (i=0;i<dialog->number_of_parameters;i++)
      {
        XtVaSetValues(dialog->spatial_toggles[i],
          XmNwidth,max_spatial_width,
          NULL);
        XtVaSetValues(dialog->values[i],
          XmNwidth,max_value_width,
          NULL);
        XtVaSetValues(dialog->units[i],
          XmNwidth,max_units_width,
          NULL);
      } /* for (i...) */
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"format_parameter_dialog_widgets. "
      "Missing parameter dialog");
  }
  LEAVE;
} /* END format_parameter_dialog_widgets() */

static int create_parameter_dialog(struct Cell_component *component)
/*******************************************************************************
LAST MODIFIED : 26 August 1999

DESCRIPTION :
Create a new parameter dialog for the cell <component>
==============================================================================*/
{
  int return_code = 0;
  Atom WM_DELETE_WINDOW;
  MrmType parameter_dialog_class;
  static MrmRegisterArg callback_list[] = {
    {"identify_parameter_rowcol",(XtPointer)identify_parameter_rowcol},
    {"ok_button_callback",(XtPointer)ok_button_callback},
    {"apply_button_callback",(XtPointer)apply_button_callback},
    {"reset_button_callback",(XtPointer)reset_button_callback},
    {"cancel_button_callback",(XtPointer)cancel_button_callback},
    {"help_button_callback",(XtPointer)help_button_callback}
  }; /* callback_list */
  static MrmRegisterArg identifier_list[] = {
    {"cell_component_structure",(XtPointer)NULL},
    {"window_width",(XtPointer)NULL},
    {"window_height",(XtPointer)NULL},
    {"dialog_title",(XtPointer)NULL}
  }; /* identifier_list */
  unsigned int width,height;
#define XmNnameColour "nameColour"
#define XmCNameColour "NameColour"
#define XmNvalueColour "valueColour"
#define XmCValueColour "ValueColour"
#define XmNunitsColour "unitsColour"
#define XmCUnitsColour "UnitsColour"
#define XmNlabelColour "LabelColour"
#define XmCLabelColour "LabelColour"
  static XtResource resources[] = {
    {
      XmNnameColour,
      XmCNameColour,
      XmRPixel,
      sizeof(Pixel),
      XtOffsetOf(User_settings,name_colour),
      XmRString,
      "blue"
    },
    {
      XmNvalueColour,
      XmCValueColour,
      XmRPixel,
      sizeof(Pixel),
      XtOffsetOf(User_settings,value_colour),
      XmRString,
      "red"
    },
    {
      XmNunitsColour,
      XmCUnitsColour,
      XmRPixel,
      sizeof(Pixel),
      XtOffsetOf(User_settings,units_colour),
      XmRString,
      "DarkGreen"
    },
    {
      XmNlabelColour,
      XmCLabelColour,
      XmRPixel,
      sizeof(Pixel),
      XtOffsetOf(User_settings,label_colour),
      XmRString,
      "blue"
    }
  }; /* resources */
  User_settings *user_settings = (User_settings *)NULL;
  
  ENTER(create_parameter_dialog);
  if (component != (struct Cell_component *)NULL)
  {
    if (component->dialog != (struct Parameter_dialog *)NULL)
    {
      /* destroy any existing parameter dialog */
      parameter_dialog_destroy_callback((Widget)NULL,(XtPointer)component,
        (XtPointer)NULL);
    }
    if (MrmOpenHierarchy_base64_string(parameter_dialog_uid64,
      &parameter_dialog_hierarchy,&parameter_dialog_hierarchy_open))
    {
      if (ALLOCATE(component->dialog,struct Parameter_dialog,1))
      {
        /* initialise the structure */
        component->dialog->shell = (Widget)NULL;
        component->dialog->window = (Widget)NULL;
        component->dialog->parameter_rowcol = (Widget)NULL;
        component->dialog->labels = (Widget *)NULL;
        component->dialog->units = (Widget *)NULL;
        component->dialog->values = (Widget *)NULL;
        component->dialog->spatial_toggles = (Widget *)NULL;
        component->dialog->number_of_parameters =
          component->number_of_parameters;
        /* make the dialog shell */
        if (component->dialog->shell =
          XtVaCreatePopupShell("parameter_dialog_shell",
            xmDialogShellWidgetClass,component->cell->shell,
            XmNmwmDecorations,MWM_DECOR_ALL,
            XmNmwmFunctions,MWM_FUNC_ALL,
            XmNdeleteResponse,XmDESTROY,
            XmNtransient,FALSE,
            NULL))
        {
          /* identify the shell for the busy icon */
          create_Shell_list_item(&(component->dialog->shell),
            component->cell->user_interface);
          /* add the destroy callback */
          XtAddCallback(component->dialog->shell,XmNdestroyCallback,
            parameter_dialog_destroy_callback,(XtPointer)component);
          WM_DELETE_WINDOW=XmInternAtom(
            XtDisplay(component->dialog->shell),
            "WM_DELETE_WINDOW",FALSE);
          XmAddWMProtocolCallback(component->dialog->shell,
            WM_DELETE_WINDOW,parameter_dialog_destroy_callback,
            (XtPointer)component);
          /* register cellbacks in UIL */
          if (MrmSUCCESS == MrmRegisterNamesInHierarchy(
            parameter_dialog_hierarchy,callback_list,XtNumber(callback_list)))
          {
            /* set the identifier's values */
            width = component->cell->user_interface->screen_width/2;
            height = component->cell->user_interface->screen_height/2;
            identifier_list[0].value = (XtPointer)component;
            identifier_list[1].value = (XtPointer)width;
            identifier_list[2].value = (XtPointer)height;
            if (component->name != (char *)NULL)
            {
              identifier_list[3].value =
                (XtPointer)XmStringCreateSimple(component->name);
            }
            else
            {
              identifier_list[3].value = (XtPointer)XmStringCreateSimple("??");
            }
            /* register the identifiers */
            if (MrmSUCCESS ==
              MrmRegisterNamesInHierarchy(parameter_dialog_hierarchy,
                identifier_list,XtNumber(identifier_list)))
            {
              /* fetch the window widget */
              if (MrmSUCCESS == MrmFetchWidget(parameter_dialog_hierarchy,
                "parameter_dialog",component->dialog->shell,
                &(component->dialog->window),&parameter_dialog_class))
              {
                /* retrieve settings */
                if (ALLOCATE(user_settings,User_settings,1))
                {
                  XtVaGetApplicationResources(component->dialog->window,
                    (XtPointer)user_settings,resources,XtNumber(resources),
                    NULL);
                  if (component->parameters != (struct Cell_parameter **)NULL)
                  {
                    if (add_parameters_to_parameter_dialog(component,
                      user_settings))
                    {
                      XtManageChild(component->dialog->window);
                      XtRealizeWidget(component->dialog->shell);
                      format_parameter_dialog_widgets(component->dialog);
                      return_code = 1;
                    }
                    else
                    {
                      display_message(ERROR_MESSAGE,"create_parameter_dialog. "
                        "Unable to add parameters to the dialog");
                      DEALLOCATE(component->dialog);
                      component->dialog = (struct Parameter_dialog *)NULL;
                      return_code = 0;
                    }
                  }
                  else
                  {
                    display_message(INFORMATION_MESSAGE,
                      "create_parameter_dialog. "
                      "No parameters to display\n");
                    XtManageChild(component->dialog->window);
                    XtRealizeWidget(component->dialog->shell);
                    return_code = 1;
                  }
                }
                else
                {
                  display_message(ERROR_MESSAGE,"create_parameter_dialog. "
                    "Unable to allocate memory for the user settings");
                  DEALLOCATE(component->dialog);
                  component->dialog = (struct Parameter_dialog *)NULL;
                  return_code = 0;
                }
              }
              else
              {
                display_message(ERROR_MESSAGE,"create_parameter_dialog. "
                  "Unable to fetch the window widget");
                DEALLOCATE(component->dialog);
                component->dialog = (struct Parameter_dialog *)NULL;
                return_code = 0;
              }
            }
            else
            {
              display_message(ERROR_MESSAGE,"create_parameter_dialog. "
                "Unable to register the identifiers");
              DEALLOCATE(component->dialog);
              component->dialog = (struct Parameter_dialog *)NULL;
              return_code = 0;  
            }
          }
          else
          {
            display_message(ERROR_MESSAGE,"create_parameter_dialog. "
              "Unable to register the callbacks");
            DEALLOCATE(component->dialog);
            component->dialog = (struct Parameter_dialog *)NULL;
            return_code = 0;  
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"create_parameter_dialog. "
            "Unable to create the dialog shell for the parameter dialog");
          DEALLOCATE(component->dialog);
          component->dialog = (struct Parameter_dialog *)NULL;
          return_code = 0;          
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"create_parameter_dialog. "
          "Unable to allocate memory for the parameter dialog");
        component->dialog = (struct Parameter_dialog *)NULL;
        return_code = 0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"create_parameter_dialog. "
        "Unable to open the parameter dialog hierarchy");
      component->dialog = (struct Parameter_dialog *)NULL;
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"create_parameter_dialog. "
      "Missing component");
    component->dialog = (struct Parameter_dialog *)NULL;
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END create_parameter_dialog() */

/*
Global functions
================
*/
int bring_up_dialog_for_name(struct Cell_window *cell,char *name)
/*******************************************************************************
LAST MODIFIED : 07 September 1999

DESCRIPTION :
Find the appropriate dialog for the given <name>, and bring it up.
==============================================================================*/
{
  int return_code;
  
  ENTER(bring_up_dialog_for_name);
  USE_PARAMETER(name);
  if (cell != (struct Cell_window *)NULL)
  {
    return_code = bring_up_parameter_dialog(cell->components);
  }
  else
  {
    display_message(ERROR_MESSAGE,"bring_up_dialog_for_name. "
      "Missing cell window");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END bring_up_dialog_for_name() */

int bring_up_parameter_dialog(struct Cell_component *component)
/*******************************************************************************
LAST MODIFIED : 18 February 1999

DESCRIPTION :
It the component's dialog exists, bring it up, else create a new dialog.
==============================================================================*/
{
  int return_code = 0;
  
  ENTER(bring_up_parameter_dialog);
  if (component != (struct Cell_component *)NULL)
  {
    if (component->dialog == (struct Parameter_dialog *)NULL)
    {
      /*display_message(INFORMATION_MESSAGE,"bring_up_parameter_dialog. "
        "creating new dialog\n");*/
      /* model dialog does not exist, so create a new one */
      if (create_parameter_dialog(component))
      {
        XtPopup(component->dialog->shell,XtGrabNone);
        return_code = 1;
      }
      else
      {
        display_message(ERROR_MESSAGE,"bring_up_parameter_dialog. "
          "Error creating the parameter dialog");
        component->dialog = (struct Parameter_dialog *)NULL;
        return_code = 0;
      }
    }
    else
    {
      /*display_message(INFORMATION_MESSAGE,"bring_up_parameter_dialog. "
        "dialog exists\n");*/
      /* dialog already exists so just pop it up */
      XtPopup(component->dialog->shell,XtGrabNone);
      return_code = 1;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"bring_up_parameter_dialog. "
      "Missing Cell window");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END bring_up_parameter_dialog() */

void close_parameter_dialog(struct Cell_component *component)
/*******************************************************************************
LAST MODIFIED : 07 February 1999

DESCRIPTION :
If the <component> has a parameter dialog, destroy it. Used when
parameters are read from file to force the dialog to be re-created.
==============================================================================*/
{
  ENTER(close_parameter_dialog);
  parameter_dialog_destroy_callback((Widget)NULL,(XtPointer)component,
    (XtPointer)NULL);
  LEAVE;
} /* END close_parameter_dialog() */

void update_parameter_dialog_boxes(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 15 September 1999

DESCRIPTION :
Updates all parameter dialog boxes, used when values are reset via a FE_node
==============================================================================*/
{
  ENTER(update_parameter_dialog_boxes);
  if (cell != (struct Cell_window *)NULL)
  {
    update_dialog_from_parameter_values(cell->components);
  }
  else
  {
    display_message(ERROR_MESSAGE,"update_parameter_dialog_boxes. "
      "Invalid arguments");
  }
  LEAVE;
} /* END update_parameter_dialog_boxes() */
