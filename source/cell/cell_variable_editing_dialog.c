/*******************************************************************************
FILE : cell_variable_editing_dialog.c

LAST MODIFIED : 11 June 2001

DESCRIPTION :
The dialog used for editing variables in Cell.
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
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Xm/Command.h>
#include <Xm/DialogS.h>
#include <Xm/MwmUtil.h>
#include <Xm/Protocols.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/TextF.h>

#include "cell/cell_plot.h"
#include "cell/cell_variable.h"
#include "cell/cell_variable_editing_dialog.h"
#include "cell/cell_variable_editing_dialog.uidh"
#include "user_interface/gui_dialog_macros.h"

/*
Module objects
--------------
*/
typedef struct
/*******************************************************************************
LAST MODIFIED : 07 November 2000

DESCRIPTION :
Holds the information related to the variable editing dialog specified in the
resource file.
==============================================================================*/
{
	Pixel name_colour,units_colour,label_colour,border_colour;
  Pixel value_colour,value_changed_colour;
  Pixel value_widget_background,value_widget_foreground;
  Dimension border_width;
} User_settings;

struct Add_widgets_iterator_user_data
/*******************************************************************************
LAST MODIFIED : 13 March 2001

DESCRIPTION :
Used to pass information to the iterator functions for adding the children
component widgets and variable widgets.
==============================================================================*/
{
  User_settings *user_settings;
  Widget parent_widget;
  struct Cell_variable_editing_dialog *dialog;
};

struct Changed_variable
/*******************************************************************************
LAST MODIFIED : 09 November 2000

DESCRIPTION :
A tempory object used to store variable information which the user changes,
until either the apply or reset callback functions are called.
==============================================================================*/
{
  /* Pointer to the variable that has changed */
  struct Cell_variable *variable;
  /**
   ** Need both the widget and the value string because the widget could be
   ** destroyed before the variable is updated and hence can't get the text
   ** field value from the widget, but if the widget is still around we want
   ** be able to set the text colour
   **/
  /* The value text field widget for the variable */
  Widget widget;
  /* The value string of the text field widget for the variable */
  char *widget_value_string;
  /* The next changed variable in the list */
  struct Changed_variable *next;
};

struct Cell_variable_widget_list
/*******************************************************************************
LAST MODIFIED : 17 January 2001

DESCRIPTION :
A structure used to keep track of a particular variable's widgets.
==============================================================================*/
{
  /* The number in the list */
  int count;
  /* The list of widgets */
  Widget *list;
}; /* struct Cell_variable_widget_list */

struct Cell_variable_editing_dialog
/*******************************************************************************
LAST MODIFIED : 07 November 2000

DESCRIPTION :
The object used to store information about the Cell variable editing dialog.
==============================================================================*/
{
  /* ??? A pointer to the main Cell interface object ??? */
  struct Cell_interface *cell_interface;
  /* The dialog shell widget */
  Widget shell;
  /* The dialog main window */
  Widget window;
  /* The row-column to put the component forms into */
  Widget rowcol;
  /* The main form of the widgets - the work window for the scrolled window and
     the parent of the above rowcol */
  Widget form;
  /* The form to hold the plotting forms */
  Widget plot_form;
  /* The plot object for this dialog */
  struct Cell_plot *plot;
  /* The list of components currently displayed in the dialog */
  struct LIST(Cell_component) *component_list;
  /* The user settings for the dialog widgets */
  User_settings *user_settings;
  /* The main user interface */
  struct User_interface *user_interface;
}; /* struct Cell_variable_editing_dialog */

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int cell_variable_editing_dialog_hierarchy_open=0;
static MrmHierarchy cell_variable_editing_dialog_hierarchy;
#endif /* defined (MOTIF) */
/* Need to keep a list of the variables that change between calls to the apply
   and reset callbacks */
static struct Changed_variable *changed_variables =
  (struct Changed_variable *)NULL;

/*
Module functions
----------------
*/
/* ?? Forward declaration ?? */
static int create_cell_component_widgets(struct Cell_component *component,
  Widget parent,User_settings *user_settings,
  struct Cell_variable_editing_dialog *dialog);
/*******************************************************************************
LAST MODIFIED : 07 November 2000

DESCRIPTION :
Creates all the widgets needed to display the <component>'s variables in the
variable editing dialog.
==============================================================================*/


DECLARE_DIALOG_IDENTIFY_FUNCTION(cell_variable_editing_dialog, \
  Cell_variable_editing_dialog,form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(cell_variable_editing_dialog, \
  Cell_variable_editing_dialog,plot_form)
  
static int invert_value_widget_colours(Widget widget,
  User_settings *user_settings)
/*******************************************************************************
LAST MODIFIED : 09 November 2000

DESCRIPTION :
Inverts the colours of the value <widget>.
==============================================================================*/
{
  int return_code = 0;

  ENTER(invert_value_widget_colours);
  if (widget && user_settings)
  {
    XtVaSetValues(widget,
      XmNforeground,user_settings->value_widget_background,
      XmNbackground,user_settings->value_widget_foreground,
      NULL);
  }
  else
  {
    display_message(ERROR_MESSAGE,"invert_value_widget_colours.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* invert_value_widget_colours() */

static int revert_value_widget_colours(Widget widget,
  User_settings *user_settings)
/*******************************************************************************
LAST MODIFIED : 09 November 2000

DESCRIPTION :
Reverts the colours of the value <widget> back to normal.
==============================================================================*/
{
  int return_code = 0;

  ENTER(revert_value_widget_colours);
  if (widget && user_settings)
  {
    XtVaSetValues(widget,
      XmNforeground,user_settings->value_widget_foreground,
      XmNbackground,user_settings->value_widget_background,
      NULL);
  }
  else
  {
    display_message(ERROR_MESSAGE,"revert_value_widget_colours.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* revert_value_widget_colours() */

static int DESTROY(Changed_variable)(
  struct Changed_variable **changed_variable_address)
/*******************************************************************************
LAST MODIFIED : 09 November 2000

DESCRIPTION :
Destroys a Changed_variable object.
==============================================================================*/
{
  struct Changed_variable *changed_variable;
  int return_code = 0;

  ENTER(DESTROY(Changed_variable));
  if (changed_variable_address &&
    (changed_variable = *changed_variable_address))
  {
    if (changed_variable->widget_value_string)
    {
      DEALLOCATE(changed_variable->widget_value_string);
    }
    DEALLOCATE(changed_variable);
    *changed_variable_address = (struct Changed_variable *)NULL;
  }
  else
  {
    display_message(ERROR_MESSAGE,"DESTROY(Changed_variable).  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* DESTROY(Changed_variable)() */

static struct Changed_variable *CREATE(Changed_variable)(
  struct Cell_variable *variable,Widget widget,char *widget_value_string)
/*******************************************************************************
LAST MODIFIED : 09 November 2000

DESCRIPTION :
Creates a Changed_variable object with the given field values.
==============================================================================*/
{
  struct Changed_variable *changed_variable;

  ENTER(CREATE(Changed_variable));
  if (variable && widget_value_string)
  {
    if (ALLOCATE(changed_variable,struct Changed_variable,1))
    {
      changed_variable->variable = variable;
      changed_variable->widget = (Widget)NULL;
      changed_variable->widget_value_string = (char *)NULL;
      changed_variable->next = (struct Changed_variable *)NULL;
      if (XtIsWidget(widget))
      {
        changed_variable->widget = widget;
      }
      if (ALLOCATE(changed_variable->widget_value_string,char,
        strlen(widget_value_string)+1))
      {
        strcpy(changed_variable->widget_value_string,widget_value_string);
      }
      else
      {
        display_message(ERROR_MESSAGE,"CREATE(Changed_variable).  "
          "Unable to allocate the memory for the changed variable "
          "widget value string");
        DESTROY(Changed_variable)(&changed_variable);
        changed_variable = (struct Changed_variable *)NULL;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"CREATE(Changed_variable).  "
        "Unable to allocate the memory for the changed variable");
      changed_variable = (struct Changed_variable *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Changed_variable).  "
      "Invalid argument(s)");
    changed_variable = (struct Changed_variable *)NULL;
  }
  LEAVE;
  return(changed_variable);
} /* CREATE(Changed_variable)() */

static void apply_button_cb(Widget widget,XtPointer dialog_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 January 2001

DESCRIPTION :
Apply button callback for the Cell variable editing dialog
==============================================================================*/
{
  struct Changed_variable *changed_variable,*temp;
  User_settings *user_settings;
  XtPointer user_settings_void;

  ENTER(apply_button_cb);
  USE_PARAMETER(call_data);
  USE_PARAMETER(dialog_void);
  if (widget)
  {
    XtVaGetValues(widget,
      XmNuserData,&user_settings_void,
      NULL);
    if (user_settings = (User_settings *)user_settings_void)
    {
      /* Apply all the changes stored in the changed variables list */
      if (changed_variables)
      {
        changed_variable = changed_variables;
        while (changed_variable)
        {
          if (Cell_variable_set_value_from_string(changed_variable->variable,
            changed_variable->widget_value_string))
          {
            /* If the widget is still around, re-colour it */
            if (XtIsWidget(changed_variable->widget))
            {
              /* first, put the colours back to normal */
              revert_value_widget_colours(changed_variable->widget,
                user_settings);
              /* then set the foreground to the changed variable colour */
              XtVaSetValues(changed_variable->widget,
                XmNforeground,user_settings->value_changed_colour,
                NULL);
            }
            /* Set the changed flag for the variable */
            Cell_variable_set_changed(changed_variable->variable,1);
          }
          temp = changed_variable->next;
          DESTROY(Changed_variable)(&changed_variable);
          changed_variable = temp;
        } /* while (changed_variable) */
        changed_variables = (struct Changed_variable *)NULL;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"apply_button_cb.  "
        "Missing user settings");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"apply_button_cb.  "
      "Invalid argument(s)");
  }
  LEAVE;
} /* apply_button_cb() */

static void ok_button_cb(Widget widget,XtPointer dialog_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 07 November 2000

DESCRIPTION :
OK button callback for the Cell variable editing dialog
==============================================================================*/
{
  struct Cell_variable_editing_dialog *dialog;

  ENTER(ok_button_cb);
  USE_PARAMETER(call_data);
  if (dialog = (struct Cell_variable_editing_dialog *)dialog_void)
  {
    /* Call the apply button callback */
    apply_button_cb(widget,(XtPointer)NULL,(XtPointer)NULL);
    /* And pop-down the dialog shell */
    if (dialog->shell)
    {
      XtPopdown(dialog->shell);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"ok_button_cb.  "
      "Missing cell variable editing dialog");
  }
  LEAVE;
} /* ok_button_cb() */

static void reset_button_cb(Widget widget,XtPointer dialog_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 07 November 2000

DESCRIPTION :
Reset button callback for the Cell variable editing dialog
==============================================================================*/
{
  struct Changed_variable *changed_variable,*temp;
  User_settings *user_settings;
  XtPointer user_settings_void;
  char *value_string;

  ENTER(apply_button_cb);
  USE_PARAMETER(call_data);
  USE_PARAMETER(dialog_void);
  if (widget)
  {
    XtVaGetValues(widget,
      XmNuserData,&user_settings_void,
      NULL);
    if (user_settings = (User_settings *)user_settings_void)
    {
      /* Reset all the changes stored in the changed variables list */
      if (changed_variables)
      {
        changed_variable = changed_variables;
        while (changed_variable)
        {
          if (value_string =
            Cell_variable_get_value_as_string(changed_variable->variable))
          {
            /* If the widget is still around, re-colour it and reset the
               value string */
            if (XtIsWidget(changed_variable->widget))
            {
              /* first, put the colours back to normal */
              revert_value_widget_colours(changed_variable->widget,
                user_settings);
              if (Cell_variable_get_changed(changed_variable->variable))
              {
                /* then set the foreground to the changed variable colour */
                XtVaSetValues(changed_variable->widget,
                  XmNforeground,user_settings->value_changed_colour,
                  NULL);
              }
              /* Re-set the text in the value widget */
              XmTextFieldSetString(changed_variable->widget,value_string);
            }
            DEALLOCATE(value_string);
          }
          temp = changed_variable->next;
          DESTROY(Changed_variable)(&changed_variable);
          changed_variable = temp;
        } /* while (changed_variable) */
        changed_variables = (struct Changed_variable *)NULL;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"reset_button_cb.  "
        "Missing user settings");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"reset_button_cb.  "
      "Invalid argument(s)");
  }
  LEAVE;
} /* reset_button_cb() */

static void cancel_button_cb(Widget widget,XtPointer dialog_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 07 November 2000

DESCRIPTION :
Cancel button callback for the Cell variable editing dialog
==============================================================================*/
{
  struct Cell_variable_editing_dialog *dialog;

  ENTER(cancel_button_cb);
  USE_PARAMETER(call_data);
  if (dialog = (struct Cell_variable_editing_dialog *)dialog_void)
  {
    /* Call the reset button callback to reset the variable widgets */
    reset_button_cb(widget,(XtPointer)NULL,(XtPointer)NULL);
    /* And pop-down the dialog shell */
    if (dialog->shell)
    {
      XtPopdown(dialog->shell);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"cancel_button_cb.  "
      "Missing cell variable editing dialog");
  }
  LEAVE;
} /* cancel_button_cb() */

static void help_button_cb(Widget widget,XtPointer dialog_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 07 November 2000

DESCRIPTION :
Help button callback for the Cell variable editing dialog
==============================================================================*/
{
  struct Cell_variable_editing_dialog *dialog;

  ENTER(help_button_cb);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (dialog = (struct Cell_variable_editing_dialog *)dialog_void)
  {
    USE_PARAMETER(dialog);
    display_message(INFORMATION_MESSAGE,"help_button_cb.\n");
  }
  else
  {
    display_message(ERROR_MESSAGE,"help_button_cb.  "
      "Missing cell variable editing dialog");
  }
  LEAVE;
} /* help_button_cb() */

static void close_cell_variable_editing_dialog(Widget widget,
  XtPointer cell_variable_editing_dialog_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 07 November 2000

DESCRIPTION :
Called when the Exit function is selected from the window manager menu in the
Cell variable editing dialog.
==============================================================================*/
{
  struct Cell_variable_editing_dialog *dialog;

  ENTER(close_cell_variable_editing_dialog);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
  if (dialog =
    (struct Cell_variable_editing_dialog *)cell_variable_editing_dialog_void)
  {
    if (dialog->shell)
    {
      XtPopdown(dialog->shell);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"close_cell_variable_editing_dialog. "
      "Missing Cell variable editing dialog");
  }
  LEAVE;
} /* close_cell_variable_editing_dialog() */

static Widget create_component_rowcol(Widget parent)
/*******************************************************************************
LAST MODIFIED : 07 November 2000

DESCRIPTION :
Creates all the rowcol used as the main manager for all the cell component
forms.
==============================================================================*/
{
  Widget rowcol;

  ENTER(create_component_rowcol);
  if (parent)
  {
    rowcol = XtVaCreateWidget("component_rowcol",
      xmRowColumnWidgetClass,parent,
      XmNentryAlignment,XmALIGNMENT_BEGINNING,
			XmNpacking,XmPACK_TIGHT,
			XmNorientation,XmVERTICAL,
			XmNnumColumns,1,
      NULL);
  }
  else
  {
    display_message(ERROR_MESSAGE,"create_component_rowcol.  "
      "Invalid argument(s)");
    rowcol = (Widget)NULL;
  }
  LEAVE;
  return(rowcol);
} /* create_component_rowcol() */

static void show_children_toggle_changed_cb(Widget widget,
  XtPointer parent_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 08 November 2000

DESCRIPTION :
Callback for when the show children toggle is changed in a component's
form.
==============================================================================*/
{
  Widget parent;
  Boolean set;

  ENTER(show_children_toggle_changed_cb);
  USE_PARAMETER(call_data);
  if (parent = (Widget)parent_void)
  {
    /* Get the state of the toggle */
    XtVaGetValues(widget,
      XmNset,&set,
      NULL);
    if (set)
    {
      /* want to show the children's widgets */
      XtManageChild(parent);
    }
    else
    {
      /* want to hide the children's widgets */
      XtUnmanageChild(parent);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"show_children_toggle_changed_cb.  "
      "Invalid argument(s)");
  }
  LEAVE;
} /* show_children_toggle_changed_cb() */

static void unemap_toggle_changed_cb(Widget widget,XtPointer variable_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 09 November 2000

DESCRIPTION :
Callback for when the varaible UnEmap toggle is changed in a component's
form.
==============================================================================*/
{
  struct Cell_variable *variable;
  Boolean set;

  ENTER(unemap_toggle_changed_cb);
  USE_PARAMETER(call_data);
  if (variable = (struct Cell_variable *)variable_void)
  {
    /* Get the state of the toggle */
    XtVaGetValues(widget,
      XmNset,&set,
      NULL);
    if (set)
    {
      /* Make sure that the variable has a UnEmap interface */
      if (!Cell_variable_variable_has_unemap_interface(variable))
      {
        /* Create the UnEmap interface for the variable */
        if (!Cell_variable_set_unemap_interface(variable))
        {
          /* Couldn't create the UnEmap interface, so reset the toggle */
          XtVaSetValues(widget,
            XmNset,False,
            NULL);
        }
      }
    }
    else
    {
      /* Need to destroy the variable's UnEmap interface */
      Cell_variable_destroy_unemap_interface(variable);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"unemap_toggle_changed_cb.  "
      "Invalid argument(s)");
  }
  LEAVE;
} /* unemap_toggle_changed_cb() */

static void plot_toggle_changed_cb(Widget widget,XtPointer variable_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 March 2001

DESCRIPTION :
Callback for when the variable plot toggle is changed in a component's
form.
==============================================================================*/
{
  struct Cell_variable *variable;
  struct Cell_variable_editing_dialog *dialog;
  XtPointer dialog_void;
  Boolean set;

  ENTER(plot_toggle_changed_cb);
  USE_PARAMETER(call_data);
  if (variable = (struct Cell_variable *)variable_void)
  {
    /* Get the state of the toggle */
    XtVaGetValues(widget,
      XmNset,&set,
      XmNuserData,&dialog_void,
      NULL);
    /* Can only plot the variable if it has a CMISS interface */
    if (Cell_variable_variable_has_cmiss_interface(variable))
    {
      if (dialog = (struct Cell_variable_editing_dialog *)dialog_void)
      {
        if (set)
        {
          Cell_plot_add_variable(dialog->plot,variable);
        }
        else
        {
          Cell_plot_remove_variable(dialog->plot,variable);
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"plot_toggle_changed_cb.  "
          "Missing dialog");
      }
    }
    else
    {
      /* No CMISS interface for the variable, so make sure the toggle is set
         to false */      
      XtVaSetValues(widget,
        XmNset,False,
        NULL);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"plot_toggle_changed_cb.  "
      "Invalid argument(s)");
  }
  LEAVE;
} /* plot_toggle_changed_cb() */

static int add_variable_to_changed_variables(struct Cell_variable *variable,
  Widget widget,char *widget_value_string)
/*******************************************************************************
LAST MODIFIED : 09 November 2000

DESCRIPTION :
Adds the <variable> to the list of variables to be changed.
==============================================================================*/
{
  int return_code = 0;
  struct Changed_variable *changed_variable,*temp;

  ENTER(add_variable_to_changed_variables);
  if (variable && widget && widget_value_string)
  {
    /* Allocate memory for the changed variable */
    if (changed_variable = CREATE(Changed_variable)(variable,widget,
      widget_value_string))
    {
      /* Set the last entry in the list */
      if (changed_variables)
      {
        temp = changed_variables;
        while (temp->next)
        {
          temp = temp->next;
        }
        temp->next = changed_variable;
      }
      else
      {
        changed_variables = changed_variable;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"add_variable_to_changed_variables.  "
        "Unable to allocate memory for the changed variable");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"add_variable_to_changed_variables.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* add_variable_to_changed_variables() */

static void variable_value_widget_changed_cb(Widget widget,
  XtPointer variable_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 09 November 2000

DESCRIPTION :
Called when the user has changed the data in the text widget. Processes the
data, and then changes the correct value in the variable object.
==============================================================================*/
{
  struct Cell_variable *variable;
  char *value_string,*widget_value_string,*checked_widget_value_string;
  XtPointer user_settings_void;
  User_settings *user_settings;
	
  ENTER(variable_value_widget_changed_cb);
  USE_PARAMETER(call_data);
  XtVaGetValues(widget,
    XmNuserData,&user_settings_void,
    NULL);
  if ((variable = (struct Cell_variable *)variable_void) &&
    (user_settings = (User_settings *)user_settings_void))
  {
    /* Get the string value from the value widget */
    if (widget_value_string = XmTextFieldGetString(widget))
    {
      /* Check the widget string for a valid value for the given variable */
      if (checked_widget_value_string =
        Cell_variable_check_value_string(variable,widget_value_string))
      {
        /* Get the current variable value as a string */
        if (value_string = Cell_variable_get_value_as_string(variable))
        {
          /* only do something if the value given in the text field differs
             from the variable's current value */
          if (strcmp(value_string,checked_widget_value_string))
          {
            /* Add the variable to the list of variables that have changed */
            add_variable_to_changed_variables(variable,widget,
              checked_widget_value_string);
            /* Invert the colours to give user a visual indication that the
               value of the variable has changed */
            invert_value_widget_colours(widget,user_settings);
          }
          DEALLOCATE(value_string);
        }
        if (strcmp(widget_value_string,checked_widget_value_string))
        {
          /* Need to re-display the widget value string */
          XmTextFieldSetString(widget,checked_widget_value_string);
        }
        DEALLOCATE(checked_widget_value_string);
      }
      else
      {
        /* Widget did not contain a valid value for this variable, so
           re-display the actual value for the variable */
        if (value_string = Cell_variable_get_value_as_string(variable))
        {
          XmTextFieldSetString(widget,value_string);
          DEALLOCATE(value_string);
        }
      }
      XtFree(widget_value_string);
    } /* if (widget_value_string = XmTextFieldGetString(widget)) */
  }
  else
  {
    display_message(ERROR_MESSAGE,"variable_value_widget_changed_cb.  "
      "Invalid argument(s)");
  }
  LEAVE;
} /* variable_value_widget_changed_cb() */

static int create_cell_variable_widgets(struct Cell_variable *variable,
  Widget parent,User_settings *user_settings,
  struct Cell_variable_editing_dialog *dialog)
/*******************************************************************************
LAST MODIFIED : 08 March 2001

DESCRIPTION :
Creates all the widgets associated with a cell <variable> and sets all the
appropriate callbacks.
==============================================================================*/
{
  int return_code = 0;
  char *name;
  Widget main_rowcol,previous,toggle;
  Boolean set;
  char *value_string;
  Dimension size;

  ENTER(create_cell_variable_widgets);
  if (variable && parent && user_settings && dialog)
  {
    /* Create the variables's rowcol to hold all the widgets */
    main_rowcol = XtVaCreateManagedWidget("cell_variable_rowcol",
      xmRowColumnWidgetClass,parent,
      XmNentryAlignment,XmALIGNMENT_BEGINNING,
			XmNpacking,XmPACK_TIGHT,
			XmNorientation,XmHORIZONTAL,
			XmNnumColumns,1,
      NULL);
    /* Create the I/V, steady-state, plotting toggle */
    toggle = XtVaCreateManagedWidget("cell_variable_plot",
      xmToggleButtonWidgetClass,main_rowcol,
      XmNlabelString,XmStringCreateSimple(""),
      XmNalignment,XmALIGNMENT_BEGINNING,
      XmNset,False,
      XmNuserData,(XtPointer)dialog,
      NULL);
    /* add the value changed callback for the plot toggle */
    XtAddCallback(toggle,XmNvalueChangedCallback,plot_toggle_changed_cb,
      (XtPointer)variable);
    /* Create the UnEmap toggle with the name label -
       need to make sure that a name exists */
    name = Cell_variable_get_name(variable);
    if (!name)
    {
      ALLOCATE(name,char,strlen("BAD!")+1);
      sprintf(name,"BAD!");
    }
    if (Cell_variable_variable_has_unemap_interface(variable))
    {
      set = True;
    }
    else
    {
      set = False;
    }
    previous = XtVaCreateManagedWidget("cell_variable_name",
      xmToggleButtonWidgetClass,main_rowcol,
      XmNlabelString,XmStringCreateSimple(name),
      XmNalignment,XmALIGNMENT_BEGINNING,
      XmNset,set,
      XmNforeground,user_settings->name_colour,
      NULL);
    DEALLOCATE(name);
    /* add the value changed callback for the UnEmap toggle */
    XtAddCallback(previous,XmNvalueChangedCallback,unemap_toggle_changed_cb,
      (XtPointer)variable);
    /* Fix up the size of the plot toggle button */
    XtVaGetValues(previous,
      XmNindicatorSize,&size,
      NULL);
    XtVaSetValues(toggle,
      XmNindicatorSize,size,
      NULL);
    /* Add the text field for the value of the variable */
    value_string = Cell_variable_get_value_as_string(variable);
    previous = XtVaCreateManagedWidget("cell_variable_text",
      xmTextFieldWidgetClass,main_rowcol,
      XmNvalue,value_string,
      XmNeditable,True,
      XmNcolumns,15,
      XmNforeground,user_settings->value_colour,
      XmNuserData,(XtPointer)user_settings,
      NULL);
    if (value_string) DEALLOCATE(value_string);
    /* Grab the foreground and background colours for later use */
    XtVaGetValues(previous,
      XmNforeground,&(user_settings->value_widget_foreground),
      XmNbackground,&(user_settings->value_widget_background),
      NULL);
    /* Change the foreground colour for variables which have changed from the
       the CellML definition */
    if (Cell_variable_get_changed(variable))
    {
      XtVaSetValues(previous,
        XmNforeground,user_settings->value_changed_colour,
        NULL);
    }
    /* add the callback to check the entries into the text field */
    XtAddCallback(previous,XmNactivateCallback,
      variable_value_widget_changed_cb,(XtPointer)variable);
    XtAddCallback(previous,XmNlosingFocusCallback,
      variable_value_widget_changed_cb,(XtPointer)variable);
    /* Add the text field to the variable's list */
    Cell_variable_add_value_text_field(variable,(void *)previous);
    /* If a display name exists for the variable, put it in */
    if (name = Cell_variable_get_display_name(variable))
    {
      XtVaCreateManagedWidget("cell_variable_display_name",
        xmLabelWidgetClass,main_rowcol,
        XmNlabelString,XmStringCreateSimple(name),
        XmNalignment,XmALIGNMENT_BEGINNING,
        NULL);
      DEALLOCATE(name);
    }
    return_code = 1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"create_cell_variable_widgets.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* create_cell_variable_widgets() */

static int add_variable_to_dialog(struct Cell_variable *variable,
  void *user_data_void)
/*******************************************************************************
LAST MODIFIED : 08 November 2000

DESCRIPTION :
Iterator function used to add the component's variable's to the variable editing
dialog.
==============================================================================*/
{
  int return_code = 0;
  struct Add_widgets_iterator_user_data *user_data;

  ENTER(add_variable_to_dialog);
  if (variable &&
    (user_data = (struct Add_widgets_iterator_user_data *)user_data_void))
  {
    return_code = create_cell_variable_widgets(variable,
      user_data->parent_widget,user_data->user_settings,user_data->dialog);
  }
  else
  {
    display_message(ERROR_MESSAGE,"add_variable_to_dialog.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* add_variable_to_dialog() */

static int add_child_component_to_dialog(struct Cell_component *component,
  void *user_data_void)
/*******************************************************************************
LAST MODIFIED : 08 November 2000

DESCRIPTION :
Iterator function used to add the children component's to the variable editing
dialog.
==============================================================================*/
{
  int return_code = 0;
  struct Add_widgets_iterator_user_data *user_data;

  ENTER(add_child_component_to_dialog);
  if (component &&
    (user_data = (struct Add_widgets_iterator_user_data *)user_data_void))
  {
    return_code = create_cell_component_widgets(component,
      user_data->parent_widget,user_data->user_settings,user_data->dialog);
  }
  else
  {
    display_message(ERROR_MESSAGE,"add_child_component_to_dialog.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* add_child_component_to_dialog() */

static int create_cell_component_widgets(struct Cell_component *component,
  Widget parent,User_settings *user_settings,
  struct Cell_variable_editing_dialog *dialog)
/*******************************************************************************
LAST MODIFIED : 13 March 2001

DESCRIPTION :
Creates all the widgets needed to display the <component>'s variables in the
variable editing dialog.
==============================================================================*/
{
  int return_code = 0;
  char *name;
  Widget main_rowcol,toggle,children_rowcol;
  struct Add_widgets_iterator_user_data user_data;
  struct LIST(Cell_variable) *variable_list;

  ENTER(create_cell_component_widgets);
  if (component && parent && user_settings && dialog)
  {
    /* Create the component's form to hold all the widgets */
    main_rowcol = XtVaCreateManagedWidget("cell_component_rowcol",
      xmRowColumnWidgetClass,parent,
      XmNentryAlignment,XmALIGNMENT_BEGINNING,
			XmNpacking,XmPACK_TIGHT,
			XmNorientation,XmVERTICAL,
			XmNnumColumns,1,
      XmNborderWidth,user_settings->border_width,
      XmNborderColor,user_settings->border_colour,
      NULL);
    /* Create the name label - eventually need to start using different font
       lists - need to make sure that a name exists */
    name = Cell_component_get_name(component);
    if (name)
    {
      if (!strcmp(name,ROOT_ELEMENT_ID))
      {
        sprintf(name,"Root Component");
      }
    }
    else
    {
      ALLOCATE(name,char,strlen("No component name - BAD!")+1);
      sprintf(name,"No component name - BAD!");
    }
    XtVaCreateManagedWidget("cell_component_name",
      xmLabelWidgetClass,main_rowcol,
      XmNlabelString,XmStringCreateSimple(name),
      XmNalignment,XmALIGNMENT_BEGINNING,
      NULL);
    DEALLOCATE(name);
    /* If a display name exists for the component, put it in */
    if (name = Cell_component_get_display_name(component))
    {
      XtVaCreateManagedWidget("cell_component_display_name",
        xmLabelWidgetClass,main_rowcol,
        XmNlabelString,XmStringCreateSimple(name),
        XmNalignment,XmALIGNMENT_BEGINNING,
        NULL);
      DEALLOCATE(name);
    }
    /* If the component has child components, enable the "show children"
       toggle (later, to get it in the right position) */
    toggle = XtVaCreateManagedWidget("cell_component_toggle",
      xmToggleButtonWidgetClass,main_rowcol,
      XmNlabelString,XmStringCreateSimple("Show child components"),
      XmNset,False,
      NULL);
    /* Add all the widgets for each of the component's variables */
    if (variable_list = Cell_component_get_variable_list(component))
    {
      /* Add a separator */
      XtVaCreateManagedWidget("cell_component_separator",
        xmSeparatorWidgetClass,main_rowcol,
        XmNmargin,(Dimension)50,
        XmNorientation,XmHORIZONTAL,
        XmNseparatorType,XmSHADOW_ETCHED_IN,
        NULL);
      user_data.user_settings = user_settings;
      user_data.parent_widget = main_rowcol;
      user_data.dialog = dialog;
      FOR_EACH_OBJECT_IN_LIST(Cell_variable)(add_variable_to_dialog,
        (void *)(&user_data),variable_list);
    }
    /* Set-up the "show children" toggle */
    if (Cell_component_component_has_children(component))
    {
      XtSetSensitive(toggle,True);
      /* Create a rowcol to hold the children components when they are
         created */
      children_rowcol = create_component_rowcol(main_rowcol);
      /* Add the callback for the "show children" toggle */
      XtAddCallback(toggle,XmNvalueChangedCallback,
        show_children_toggle_changed_cb,(XtPointer)children_rowcol);
      /* And create all the child components widgets */
      user_data.user_settings = user_settings;
      user_data.parent_widget = children_rowcol;
      user_data.dialog = dialog;
      FOR_EACH_OBJECT_IN_LIST(Cell_component)(add_child_component_to_dialog,
        (void *)(&user_data),Cell_component_get_children_list(component));
    }
    else
    {
      XtSetSensitive(toggle,False);
    }
    return_code = 1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"create_cell_component_widgets.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* create_cell_component_widgets() */

/*
Global functions
----------------
*/
struct Cell_variable_editing_dialog *CREATE(Cell_variable_editing_dialog)(
  struct Cell_interface *cell_interface,Widget parent,
  struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 13 March 2001

DESCRIPTION :
Create a variable editing dialog.
==============================================================================*/
{
  struct Cell_variable_editing_dialog *cell_variable_editing_dialog;
  Atom WM_DELETE_WINDOW;
  MrmType cell_variable_editing_dialog_class;
  static MrmRegisterArg callback_list[] = {
    {"id_component_form",
     (XtPointer)DIALOG_IDENTIFY(cell_variable_editing_dialog,form)},
    {"id_plot_form",
     (XtPointer)DIALOG_IDENTIFY(cell_variable_editing_dialog,plot_form)},
    {"ok_button_callback",(XtPointer)ok_button_cb},
    {"apply_button_callback",(XtPointer)apply_button_cb},
    {"reset_button_callback",(XtPointer)reset_button_cb},
    {"cancel_button_callback",(XtPointer)cancel_button_cb},
    {"help_button_callback",(XtPointer)help_button_cb}
  }; /* callback_list */
  static MrmRegisterArg identifier_list[] = {
    {"variable_editing_dialog",(XtPointer)NULL},
    {"window_width",(XtPointer)NULL},
    {"window_height",(XtPointer)NULL},
    {"dialog_title",(XtPointer)NULL},
    {"user_settings_void",(XtPointer)NULL}
  }; /* identifier_list */
  unsigned int width,height;
#define XmNnameColour "nameColour"
#define XmCNameColour "NameColour"
#define XmNvalueColour "valueColour"
#define XmCValueColour "ValueColour"
#define XmNvalueChangedColour "valueChangedColour"
#define XmCValueChangedColour "ValueChangedColour"
#define XmNunitsColour "unitsColour"
#define XmCUnitsColour "UnitsColour"
#define XmNlabelColour "LabelColour"
#define XmCLabelColour "LabelColour"
#define XmNborderColour "BorderColour"
#define XmCBorderColour "BorderColour"
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
      XmNvalueChangedColour,
      XmCValueChangedColour,
      XmRPixel,
      sizeof(Pixel),
      XtOffsetOf(User_settings,value_changed_colour),
      XmRString,
      "DarkGreen"
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
    },
    {
      XmNborderColour,
      XmCBorderColour,
      XmRPixel,
      sizeof(Pixel),
      XtOffsetOf(User_settings,border_colour),
      XmRString,
      "red"
    },
    {
      XmNborderWidth,
      XmCBorderWidth,
      XmRDimension,
      sizeof(Dimension),
      XtOffsetOf(User_settings,border_width),
      XmRString,
      "2"
    }
  }; /* resources */
  
  ENTER(CREATE(Cell_variable_editing_dialog));
  if (cell_interface && parent && user_interface)
  {
    if (MrmOpenHierarchy_base64_string(cell_variable_editing_dialog_uidh,
      &cell_variable_editing_dialog_hierarchy,
      &cell_variable_editing_dialog_hierarchy_open))
    {
      if (ALLOCATE(cell_variable_editing_dialog,
        struct Cell_variable_editing_dialog,1))
      {
        /* Initialise the structure */
        cell_variable_editing_dialog->cell_interface = cell_interface;
        cell_variable_editing_dialog->user_interface = user_interface;
        cell_variable_editing_dialog->shell = (Widget)NULL;
        cell_variable_editing_dialog->window = (Widget)NULL;
        cell_variable_editing_dialog->form = (Widget)NULL;
        cell_variable_editing_dialog->plot_form = (Widget)NULL;
        cell_variable_editing_dialog->plot = (struct Cell_plot *)NULL;
        cell_variable_editing_dialog->rowcol = (Widget)NULL;
        cell_variable_editing_dialog->component_list =
          (struct LIST(Cell_component) *)NULL;
        /* Make the dialog shell */
        if (cell_variable_editing_dialog->shell =
          XtVaCreatePopupShell("cell_variable_editing_dialog_shell",
            xmDialogShellWidgetClass,parent,
            XmNmwmDecorations,MWM_DECOR_ALL,
            XmNmwmFunctions,MWM_FUNC_ALL,
            XmNdeleteResponse,XmDO_NOTHING,
            XmNtransient,FALSE,
            NULL))
        {
          /* Identify the shell for the busy icon */
          create_Shell_list_item(&(cell_variable_editing_dialog->shell),
            user_interface);
          /* Add the destroy callback */
          XtAddCallback(cell_variable_editing_dialog->shell,XmNdestroyCallback,
            close_cell_variable_editing_dialog,
            (XtPointer)cell_variable_editing_dialog);
          WM_DELETE_WINDOW=XmInternAtom(
            XtDisplay(cell_variable_editing_dialog->shell),
            "WM_DELETE_WINDOW",FALSE);
          XmAddWMProtocolCallback(cell_variable_editing_dialog->shell,
            WM_DELETE_WINDOW,close_cell_variable_editing_dialog,
            (XtPointer)cell_variable_editing_dialog);
          /* register cellbacks in UIL */
          if (MrmSUCCESS == MrmRegisterNamesInHierarchy(
            cell_variable_editing_dialog_hierarchy,callback_list,
            XtNumber(callback_list)))
          {
            /* Need to allocate here so can use the pointer to the user
               settings */
            if (ALLOCATE(cell_variable_editing_dialog->user_settings,
              User_settings,1))
            {
              /* Set the identifier's values */
              width = 500;
              height = 400;
              /* the pointer to this dialog */
              identifier_list[0].value =
                (XtPointer)cell_variable_editing_dialog;
              /* the dialog width and height */
              identifier_list[1].value = (XtPointer)width;
              identifier_list[2].value = (XtPointer)height;
              /* the dialog title */
              identifier_list[3].value =
                (XtPointer)XmStringCreateSimple("Cell variable editing");
              /* the user settings */
              identifier_list[4].value =
                (XtPointer)(cell_variable_editing_dialog->user_settings);
              /* Register the identifiers */
              if (MrmSUCCESS ==
                MrmRegisterNamesInHierarchy(
                  cell_variable_editing_dialog_hierarchy,
                  identifier_list,XtNumber(identifier_list)))
              {
                /* Fetch the window widget */
                if (MrmSUCCESS == MrmFetchWidget(
                  cell_variable_editing_dialog_hierarchy,
                  "variable_editing_dialog_widget",
                  cell_variable_editing_dialog->shell,
                  &(cell_variable_editing_dialog->window),
                  &cell_variable_editing_dialog_class))
                {
                  /* Retrieve settings */
                  XtVaGetApplicationResources(
                    cell_variable_editing_dialog->window,
                    (XtPointer)(cell_variable_editing_dialog->user_settings),
                    resources,XtNumber(resources),NULL);
                  /* Create the plotting object */
                  if (cell_variable_editing_dialog->plot_form)
                  {
                    if (cell_variable_editing_dialog->plot = CREATE(Cell_plot)(
                      cell_variable_editing_dialog->plot_form))
                    {
                      XtManageChild(cell_variable_editing_dialog->window);
                      XtRealizeWidget(cell_variable_editing_dialog->shell);
                      /* All the widgets created, so create the data objects for
                         the dialog */
                      if (cell_variable_editing_dialog->component_list =
                        CREATE(LIST(Cell_component))())
                      {
                        /* Set-up the initial sizes of the plotting drawing
                           area's */
                        Cell_plot_set_pane_sizes(
                          cell_variable_editing_dialog->plot,
                          cell_variable_editing_dialog->window);
                      }
                      else
                      {
                        display_message(ERROR_MESSAGE,
                          "CREATE(Cell_variable_editing_dialog). "
                          "Unable to create the component list object");
                        DESTROY(Cell_variable_editing_dialog)(
                          &cell_variable_editing_dialog);
                        cell_variable_editing_dialog =
                          (struct Cell_variable_editing_dialog *)NULL;
                      }
                    }
                    else
                    {
                      display_message(ERROR_MESSAGE,
                        "CREATE(Cell_variable_editing_dialog). "
                        "Unable to create the plot object");
                      DESTROY(Cell_variable_editing_dialog)(
                        &cell_variable_editing_dialog);
                      cell_variable_editing_dialog =
                        (struct Cell_variable_editing_dialog *)NULL;
                    }
                  }
                  else
                  {
                    display_message(ERROR_MESSAGE,
                      "CREATE(Cell_variable_editing_dialog). "
                      "Missing plot form");
                    DESTROY(Cell_variable_editing_dialog)(
                      &cell_variable_editing_dialog);
                    cell_variable_editing_dialog =
                      (struct Cell_variable_editing_dialog *)NULL;
                  }
                }
                else
                {
                  display_message(ERROR_MESSAGE,
                    "CREATE(Cell_variable_editing_dialog). "
                    "Unable to fetch the window widget");
                  DESTROY(Cell_variable_editing_dialog)(
                    &cell_variable_editing_dialog);
                  cell_variable_editing_dialog =
                    (struct Cell_variable_editing_dialog *)NULL;
                }
              }
              else
              {
                display_message(ERROR_MESSAGE,
                  "CREATE(Cell_variable_editing_dialog). "
                  "Unable to register the identifiers");
                DESTROY(Cell_variable_editing_dialog)(
                  &cell_variable_editing_dialog);
                cell_variable_editing_dialog =
                  (struct Cell_variable_editing_dialog *)NULL;
              }
            }
            else
            {
              display_message(ERROR_MESSAGE,
                "CREATE(Cell_variable_editing_dialog). "
                "Unable to allocate memory for the user settings");
              DESTROY(Cell_variable_editing_dialog)(
                &cell_variable_editing_dialog);
              cell_variable_editing_dialog =
                (struct Cell_variable_editing_dialog *)NULL;
            }
          }
          else
          {
            display_message(ERROR_MESSAGE,
              "CREATE(Cell_variable_editing_dialog). "
              "Unable to register the callbacks");
            DESTROY(Cell_variable_editing_dialog)(
              &cell_variable_editing_dialog);
            cell_variable_editing_dialog =
              (struct Cell_variable_editing_dialog *)NULL;
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"CREATE(Cell_variable_editing_dialog). "
            "Unable to create the dialog shell");
          DESTROY(Cell_variable_editing_dialog)(&cell_variable_editing_dialog);
          cell_variable_editing_dialog =
            (struct Cell_variable_editing_dialog *)NULL;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"CREATE(Cell_variable_editing_dialog). "
          "Unable to allocate memory for the dialog");
        cell_variable_editing_dialog =
          (struct Cell_variable_editing_dialog *)NULL;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"CREATE(Cell_variable_editing_dialog). "
        "Unable to open the dialog hierarchy");
      cell_variable_editing_dialog =
        (struct Cell_variable_editing_dialog *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Cell_variable_editing_dialog). "
      "Invalid argument(s)");
    cell_variable_editing_dialog = (struct Cell_variable_editing_dialog *)NULL;
  }
  LEAVE;
  return(cell_variable_editing_dialog);
} /* CREATE(Cell_variable_editing_dialog)() */

int DESTROY(Cell_variable_editing_dialog)(
  struct Cell_variable_editing_dialog **dialog_address)
/*******************************************************************************
LAST MODIFIED : 13 March 2001

DESCRIPTION :
Destroys the Cell_variable_editing_dialog object.
==============================================================================*/
{
  int return_code;
  struct Cell_variable_editing_dialog *dialog;
  
  ENTER(DESTROY(Cell_variable_editing_dialog));
  if (dialog_address && (dialog = *dialog_address))
	{
    /* make sure the window isn't currently being displayed */
    if (dialog->shell && dialog->window)
    {
      XtPopdown(dialog->shell);
      /* remove the dialog from the shell list */
      destroy_Shell_list_item_from_shell(&(dialog->shell),
        dialog->user_interface);
      /* destroying the window should destroy all the children */
      XtDestroyWidget(dialog->window);
      /* Need to clear all the widget references as well */
      FOR_EACH_OBJECT_IN_LIST(Cell_variable)(
        Cell_variable_delete_value_text_fields,
        (void *)NULL,Cell_interface_get_variable_list(dialog->cell_interface));
    }
    /* empty the component list */
    if (dialog->component_list)
    {
      REMOVE_ALL_OBJECTS_FROM_LIST(Cell_component)(dialog->component_list);
      DESTROY(LIST(Cell_component))(&(dialog->component_list));
    }
    /* Destroy the plotting object */
    if (dialog->plot)
    {
      DESTROY(Cell_plot)(&(dialog->plot));
    }
		DEALLOCATE(*dialog_address);
    *dialog_address = (struct Cell_variable_editing_dialog *)NULL;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Cell_variable_editing_dialog).  Invalid argument(s)");
		return_code=0;
	}
  LEAVE;
  return(return_code);
} /* DESTROY(Cell_variable_editing_dialog)() */

int Cell_variable_editing_dialog_pop_up(
  struct Cell_variable_editing_dialog *dialog)
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Pops up the variable editing dialog
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_variable_editing_dialog_pop_up);
  if (dialog)
  {
    if (dialog->shell)
    {
      XtPopup(dialog->shell,XtGrabNone);
      XtVaSetValues(dialog->shell,
        XmNiconic,False,
        NULL);
    }
    return_code = 1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_variable_editing_dialog_pop_up.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_variable_editing_dialog_pop_up() */

int Cell_variable_editing_dialog_pop_down(
  struct Cell_variable_editing_dialog *dialog)
/*******************************************************************************
LAST MODIFIED : 22 January 2001

DESCRIPTION :
Pops down the variable editing dialog
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_variable_editing_dialog_pop_down);
  if (dialog)
  {
    if (dialog->shell)
    {
      XtPopdown(dialog->shell);
    }
    return_code = 1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_variable_editing_dialog_pop_up.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_variable_editing_dialog_pop_up() */

int Cell_variable_editing_dialog_add_component(
  struct Cell_variable_editing_dialog *dialog,
  struct Cell_component *cell_component)
/*******************************************************************************
LAST MODIFIED : 07 November 2000

DESCRIPTION :
Adds the given <cell_component> to the variable editing <dialog>.
==============================================================================*/
{
  int return_code = 0;
  char *name;

  ENTER(Cell_variable_editing_dialog_add_component);
  if (dialog && cell_component)
  {
    /* Get the name of the component */
    if (name = Cell_component_get_name(cell_component))
    {
      /* Check if the component is already in the dialog */
      if (!(FIND_BY_IDENTIFIER_IN_LIST(Cell_component,name)(name,
        dialog->component_list)))
      {
        /* Add the component to the dialog's list */
        if (ADD_OBJECT_TO_LIST(Cell_component)(cell_component,
          dialog->component_list))
        {
          /* Make sure that the rowcol exisits */
          if (!(dialog->rowcol))
          {
            dialog->rowcol = create_component_rowcol(dialog->form);
            XtManageChild(dialog->rowcol);
          }
          /* Now create the widgets for the component */
          return_code = create_cell_component_widgets(cell_component,
            dialog->rowcol,dialog->user_settings,dialog);
        }
        else
        {
          display_message(ERROR_MESSAGE,"Cell_variable_editing_dialog.  "
            "Unable to add the component (%s) to the component list",name);
          return_code = 0;
        }
      }
      else
      {
        /* This component is already in the dialog (top level), so do not
           need to add it again */
        return_code = 1;
      }
      DEALLOCATE(name);
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "Cell_variable_editing_dialog_add_component.  "
        "Unable to get the name of the component");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_variable_editing_dialog_add_component.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_variable_editing_dialog_add_component() */

int Cell_variable_editing_dialog_clear_dialog(
  struct Cell_variable_editing_dialog *dialog)
/*******************************************************************************
LAST MODIFIED : 07 November 2000

DESCRIPTION :
Removes all the component widgets from the variable editing dialog.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_variable_editing_dialog_clear_dialog);
  if (dialog)
  {
    if (dialog->rowcol)
    {
      XtUnmanageChild(dialog->rowcol);
      XtDestroyWidget(dialog->rowcol);
      /* Need to clear all the widget references as well */
      FOR_EACH_OBJECT_IN_LIST(Cell_variable)(
        Cell_variable_delete_value_text_fields,
        (void *)NULL,Cell_interface_get_variable_list(dialog->cell_interface));
      dialog->rowcol = (Widget)NULL;
    }
    /* Remove all the components from the list */
    REMOVE_ALL_OBJECTS_FROM_LIST(Cell_component)(dialog->component_list);
    return_code = 1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_variable_editing_dialog_clear_dialog.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_variable_editing_dialog_clear_dialog() */

struct Cell_variable_widget_list *CREATE(Cell_variable_widget_list)(void)
/*******************************************************************************
LAST MODIFIED : 17 January 2001

DESCRIPTION :
Create a new widget list object
==============================================================================*/
{
  struct Cell_variable_widget_list *widget_list;

  ENTER(CREATE(Cell_variable_widget_list));
  if (ALLOCATE(widget_list,struct Cell_variable_widget_list,1))
  {
    widget_list->count = 0;
    widget_list->list = (Widget *)NULL;
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Cell_variable_widget_list).  "
      "Invalid argument(s)");
    widget_list = (struct Cell_variable_widget_list *)NULL;
  }
  LEAVE;
  return(widget_list);
} /* CREATE(Cell_variable_widget_list)() */

int DESTROY(Cell_variable_widget_list)(
  struct Cell_variable_widget_list **list_address)
/*******************************************************************************
LAST MODIFIED : 17 January 2001

DESCRIPTION :
Destroys the Cell_variable_widget_list object.
==============================================================================*/
{
  int return_code;
  struct Cell_variable_widget_list *list;
  
  ENTER(DESTROY(Cell_variable_widget_list));
  if (list_address && (list = *list_address))
	{
    if (list->list)
    {
      DEALLOCATE(list->list);
    }
		DEALLOCATE(*list_address);
    *list_address = (struct Cell_variable_widget_list *)NULL;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Cell_variable_widget_list).  Invalid argument(s)");
		return_code=0;
	}
  LEAVE;
  return(return_code);
} /* DESTROY(Cell_variable_widget_list)() */

int Cell_variable_widget_list_add_widget(
  struct Cell_variable_widget_list *list,void *widget_void)
/*******************************************************************************
LAST MODIFIED : 17 January 2001

DESCRIPTION :
Adds the given <widget> to the <list>.
==============================================================================*/
{
  int return_code = 0;
  Widget widget;

  ENTER(Cell_variable_widget_list_add_widget);
  if (list && widget_void && (widget = (Widget)widget_void))
  {
    if (list->count == 0)
    {
      list->count = 1;
      if (ALLOCATE(list->list,Widget,1))
      {
        list->list[0] = widget;
        return_code = 1;
      }
      else
      {
        display_message(ERROR_MESSAGE,"Cell_variable_widget_list_add_widget.  "
          "Unable to allocate memory for the widget list");
        return_code = 0;
      }
    }
    else
    {
      list->count++;
      if (REALLOCATE(list->list,list->list,Widget,list->count))
      {
        list->list[(list->count)-1] = widget;
        return_code = 1;
      }
      else
      {
        display_message(ERROR_MESSAGE,"Cell_variable_widget_list_add_widget.  "
          "Unable to reallocate memory for the widget list");
        return_code = 0;
      }
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_variable_widget_list_add_widget.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_variable_widget_list_add_widget() */

int Cell_variable_widget_list_update_text_field_value(
  struct Cell_variable_widget_list *list,char *value_string)
/*******************************************************************************
LAST MODIFIED : 19 January 2001

DESCRIPTION :
Sets the value of all the text field widgets in the widget <list> to the given
<value_string>
==============================================================================*/
{
  int return_code = 0,i;
  Widget widget;

  ENTER(Cell_variable_widget_list_update_text_field_value);
  if (list && value_string)
  {
    for (i=0;i<list->count;i++)
    {
      widget = list->list[i];
      if (XtIsWidget(widget) && XmIsTextField(widget))
      {
        XmTextFieldSetString(widget,value_string);
      }
    }
    return_code = 1;
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_variable_widget_list_update_text_field_value.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_variable_widget_list_update_text_field_value() */
