/*******************************************************************************
FILE : cell_calculate_dialog.c

LAST MODIFIED : 21 February 2001

DESCRIPTION :
Routines for the model calculation dialog
==============================================================================*/
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Xm/Command.h>
#include <Xm/DialogS.h>
#include <Xm/PushB.h>
#include <Xm/MwmUtil.h>
#include <Xm/Protocols.h>
#include <Xm/TextF.h>
#include <Mrm/MrmPublic.h>

#include "cell/cell_calculate_dialog.h"
#include "cell/cell_calculate_dialog.uidh"
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/user_interface.h"

/*
Module objects
--------------
*/
typedef struct
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Holds the information related to the calculate dialog specified in the
resource file.
==============================================================================*/
{
	Pixel name_colour,label_colour,value_colour;
} User_settings;

struct Cell_calculate_dialog
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
The object for the model calculation dialog
==============================================================================*/
{
  /* Always need a handle on the calculation object */
  struct Cell_calculate *calculate;
  /* The dialog shell widget */
  Widget shell;
  /* The dialog main window */
  Widget window;
  /* The option menu for the model routine selection */
  Widget model_routine_option_menu;
  /* The pulldown menu for the model routine selection option menu */
  Widget model_routine_pulldown;
  /* The model routine text field */
  Widget model_routine_textfield;
  /* The model DSO text field */
  Widget model_dso_textfield;
  /* The option menu for the integrator routine selection */
  Widget intg_routine_option_menu;
  /* The pulldown menu for the integrator routine selection option menu */
  Widget intg_routine_pulldown;
  /* The integrator routine text field */
  Widget intg_routine_textfield;
  /* The integrator DSO text field */
  Widget intg_dso_textfield;
  /* The user settings for the dialog widgets */
  User_settings *user_settings;
  /* The main user interface */
  struct User_interface *user_interface;
  /* The main Cell interface */
  struct Cell_interface *interface;
}; /* struct Cell_calculate_dialog */

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int cell_calculate_dialog_hierarchy_open=0;
static MrmHierarchy cell_calculate_dialog_hierarchy;
#endif /* defined (MOTIF) */

/* The built-in model routines */
static int number_of_built_in_models = 3;
static char *built_in_models_routines[] = {
  "bob_1",
  "Joe_Smith",
  "andres_cell_model"
};
/* and the user defined switch */
#define USER_DEFINED_MODEL_STRING "User Defined Model"
/* The built-in integration routines */
static int number_of_built_in_intgs = 3;
static char *built_in_intgs_routines[] = {
  "euler",
  "improved_euler",
  "runge_kutta"
};
/* and the user defined switch */
#define USER_DEFINED_INTG_STRING "User Defined Integrator"

/*
Module functions
----------------
*/
DECLARE_DIALOG_IDENTIFY_FUNCTION(cell_calculate_dialog,Cell_calculate_dialog, \
  model_routine_pulldown)
DECLARE_DIALOG_IDENTIFY_FUNCTION(cell_calculate_dialog,Cell_calculate_dialog, \
  model_routine_textfield)
DECLARE_DIALOG_IDENTIFY_FUNCTION(cell_calculate_dialog,Cell_calculate_dialog, \
  model_dso_textfield)
DECLARE_DIALOG_IDENTIFY_FUNCTION(cell_calculate_dialog,Cell_calculate_dialog, \
  model_routine_option_menu)
DECLARE_DIALOG_IDENTIFY_FUNCTION(cell_calculate_dialog,Cell_calculate_dialog, \
  intg_routine_pulldown)
DECLARE_DIALOG_IDENTIFY_FUNCTION(cell_calculate_dialog,Cell_calculate_dialog, \
  intg_routine_textfield)
DECLARE_DIALOG_IDENTIFY_FUNCTION(cell_calculate_dialog,Cell_calculate_dialog, \
  intg_dso_textfield)
DECLARE_DIALOG_IDENTIFY_FUNCTION(cell_calculate_dialog,Cell_calculate_dialog, \
  intg_routine_option_menu)
  
static void tstart_changed_cb(Widget widget,XtPointer dialog_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Callback for when the tstart value widget changes.
==============================================================================*/
{
  struct Cell_calculate_dialog *dialog;
  char *value_string,*widget_value_string;

  ENTER(tstart_changed_cb);
  USE_PARAMETER(call_data);
  if (dialog = (struct Cell_calculate_dialog *)dialog_void)
  {
    /* Get the value string from the widget */
    if (widget_value_string = XmTextFieldGetString(widget))
    {
      /* Check the widget string for a valid value and update the value */
      if (value_string =
        Cell_calculate_set_start_time_from_string(dialog->calculate,
          widget_value_string))
      {
        /* Only update the text field if the value changes */
        if (strcmp(value_string,widget_value_string))
        {
          XmTextFieldSetString(widget,value_string);
        }
        DEALLOCATE(value_string);
      }
      else
      {
        /* The modified value is not valid, so redisplay the current value */
        if (value_string =
          Cell_calculate_get_start_time_as_string(dialog->calculate))
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
    display_message(ERROR_MESSAGE,"tstart_changed_cb.  "
      "Invalid argument(s)");
  }
  LEAVE;
} /* tstart_changed_cb() */

static void tend_changed_cb(Widget widget,XtPointer dialog_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Callback for when the tend value widget changes.
==============================================================================*/
{
  struct Cell_calculate_dialog *dialog;
  char *value_string,*widget_value_string;

  ENTER(tend_changed_cb);
  USE_PARAMETER(call_data);
  if (dialog = (struct Cell_calculate_dialog *)dialog_void)
  {
    /* Get the value string from the widget */
    if (widget_value_string = XmTextFieldGetString(widget))
    {
      /* Check the widget string for a valid value and update the value */
      if (value_string =
        Cell_calculate_set_end_time_from_string(dialog->calculate,
          widget_value_string))
      {
        /* Only update the text field if the value changes */
        if (strcmp(value_string,widget_value_string))
        {
          XmTextFieldSetString(widget,value_string);
        }
        DEALLOCATE(value_string);
      }
      else
      {
        /* The modified value is not valid, so redisplay the current value */
        if (value_string =
          Cell_calculate_get_end_time_as_string(dialog->calculate))
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
    display_message(ERROR_MESSAGE,"tend_changed_cb.  "
      "Invalid argument(s)");
  }
  LEAVE;
} /* tend_changed_cb() */

static void dt_changed_cb(Widget widget,XtPointer dialog_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Callback for when the dt value widget changes.
==============================================================================*/
{
  struct Cell_calculate_dialog *dialog;
  char *value_string,*widget_value_string;

  ENTER(dt_changed_cb);
  USE_PARAMETER(call_data);
  if (dialog = (struct Cell_calculate_dialog *)dialog_void)
  {
    /* Get the value string from the widget */
    if (widget_value_string = XmTextFieldGetString(widget))
    {
      /* Check the widget string for a valid value and update the value */
      if (value_string =
        Cell_calculate_set_dt_from_string(dialog->calculate,
          widget_value_string))
      {
        /* Only update the text field if the value changes */
        if (strcmp(value_string,widget_value_string))
        {
          XmTextFieldSetString(widget,value_string);
        }
        DEALLOCATE(value_string);
      }
      else
      {
        /* The modified value is not valid, so redisplay the current value */
        if (value_string =
          Cell_calculate_get_dt_as_string(dialog->calculate))
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
    display_message(ERROR_MESSAGE,"dt_changed_cb.  "
      "Invalid argument(s)");
  }
  LEAVE;
} /* dt_changed_cb() */

static void tabt_changed_cb(Widget widget,XtPointer dialog_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Callback for when the tabt value widget changes.
==============================================================================*/
{
  struct Cell_calculate_dialog *dialog;
  char *value_string,*widget_value_string;

  ENTER(tabt_changed_cb);
  USE_PARAMETER(call_data);
  if (dialog = (struct Cell_calculate_dialog *)dialog_void)
  {
    /* Get the value string from the widget */
    if (widget_value_string = XmTextFieldGetString(widget))
    {
      /* Check the widget string for a valid value and update the value */
      if (value_string =
        Cell_calculate_set_tabt_from_string(dialog->calculate,
          widget_value_string))
      {
        /* Only update the text field if the value changes */
        if (strcmp(value_string,widget_value_string))
        {
          XmTextFieldSetString(widget,value_string);
        }
        DEALLOCATE(value_string);
      }
      else
      {
        /* The modified value is not valid, so redisplay the current value */
        if (value_string =
          Cell_calculate_get_tabt_as_string(dialog->calculate))
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
    display_message(ERROR_MESSAGE,"tabt_changed_cb.  "
      "Invalid argument(s)");
  }
  LEAVE;
} /* tabt_changed_cb() */

static void calculate_button_cb(Widget widget,XtPointer dialog_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Calculate button callback for the Cell calculate dialog
==============================================================================*/
{
  struct Cell_calculate_dialog *dialog;

  ENTER(calculate_button_cb);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (dialog = (struct Cell_calculate_dialog *)dialog_void)
  {
    if (dialog->shell)
    {
      XtPopdown(dialog->shell);
    }
    Cell_interface_calculate_model(dialog->interface);
  }
  else
  {
    display_message(ERROR_MESSAGE,"calculate_button_cb.  "
      "Missing cell calculate dialog");
  }
  LEAVE;
} /* calculate_button_cb() */

static void close_button_cb(Widget widget,XtPointer dialog_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Close button callback for the Cell calculate dialog
==============================================================================*/
{
  struct Cell_calculate_dialog *dialog;

  ENTER(close_button_cb);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (dialog = (struct Cell_calculate_dialog *)dialog_void)
  {
    if (dialog->shell && XtIsWidget(dialog->shell))
    {
      XtPopdown(dialog->shell);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"close_button_cb.  "
      "Missing cell calculate dialog");
  }
  LEAVE;
} /* close_button_cb() */

static void help_button_cb(Widget widget,XtPointer dialog_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Help button callback for the Cell calculate dialog
==============================================================================*/
{
  struct Cell_calculate_dialog *dialog;

  ENTER(help_button_cb);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (dialog = (struct Cell_calculate_dialog *)dialog_void)
  {
    USE_PARAMETER(dialog);
    display_message(INFORMATION_MESSAGE,"help_button_cb.  "
      "Help goes here!!\n");
  }
  else
  {
    display_message(ERROR_MESSAGE,"help_button_cb.  "
      "Missing cell calculate dialog");
  }
  LEAVE;
} /* help_button_cb() */

static void model_routine_changed_cb(Widget widget,XtPointer dialog_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 19 December 2000

DESCRIPTION :
Callback for when the model routine name widget changes.
==============================================================================*/
{
  struct Cell_calculate_dialog *dialog;
  char *value_string,*widget_value_string;

  ENTER(model_routine_changed_cb);
  USE_PARAMETER(call_data);
  if (dialog = (struct Cell_calculate_dialog *)dialog_void)
  {
    /* Get the value string from the widget */
    if (widget_value_string = XmTextFieldGetString(widget))
    {
      /* Check the widget string for a valid value and update the value */
      if (!Cell_calculate_set_model_routine_name(dialog->calculate,
        widget_value_string))
      {
        /* The modified value is not valid, so redisplay the current value */
        if (value_string = Cell_calculate_get_model_routine_name(
          dialog->calculate))
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
    display_message(ERROR_MESSAGE,"model_routine_changed_cb.  "
      "Invalid argument(s)");
  }
  LEAVE;
} /* model_routine_changed_cb() */

static void model_dso_changed_cb(Widget widget,XtPointer dialog_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 19 December 2000

DESCRIPTION :
Callback for when the model DSO name widget changes.
==============================================================================*/
{
  struct Cell_calculate_dialog *dialog;
  char *value_string,*widget_value_string;

  ENTER(model_dso_changed_cb);
  USE_PARAMETER(call_data);
  if (dialog = (struct Cell_calculate_dialog *)dialog_void)
  {
    /* Get the value string from the widget */
    if (widget_value_string = XmTextFieldGetString(widget))
    {
      /* Check the widget string for a valid value and update the value */
      if (!Cell_calculate_set_dso_name(dialog->calculate,
        widget_value_string))
      {
        /* The modified value is not valid, so redisplay the current value */
        if (value_string = Cell_calculate_get_dso_name(
          dialog->calculate))
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
    display_message(ERROR_MESSAGE,"model_dso_changed_cb.  "
      "Invalid argument(s)");
  }
  LEAVE;
} /* model_dso_changed_cb() */

static void model_routine_menu_cb(Widget widget,XtPointer dialog_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 19 December 2000

DESCRIPTION :
Callback for the push buttons in the model routine option menu.
==============================================================================*/
{
  struct Cell_calculate_dialog *dialog;
  XmString str;
  char *button_label;

  ENTER(model_routine_menu_cb);
  USE_PARAMETER(call_data);
  if (dialog = (struct Cell_calculate_dialog *)dialog_void)
  {
    USE_PARAMETER(dialog);
    XtVaGetValues(widget,
      XmNlabelString,&str,
      NULL);
    XmStringGetLtoR(str,XmSTRING_DEFAULT_CHARSET,&button_label);
    XmStringFree(str);
    if (strcmp(button_label,USER_DEFINED_MODEL_STRING) == 0)
    {
      /* Make the text fields editable */
      XtVaSetValues(dialog->model_routine_textfield,
        XmNsensitive,True,
        XmNeditable,True,
        XmNvalue,"",
        NULL);
      XtVaSetValues(dialog->model_dso_textfield,
        XmNsensitive,True,
        XmNeditable,True,
        XmNvalue,"",
        NULL);
    }
    else
    {
      /* Make the text fields un-editable */
      XtVaSetValues(dialog->model_routine_textfield,
        XmNvalue,button_label,
        XmNsensitive,False,
        XmNeditable,False,
        NULL);
      XtVaSetValues(dialog->model_dso_textfield,
        XmNvalue,"<N/A>",
        XmNsensitive,False,
        XmNeditable,False,
        NULL);
      /* and set the model strings */
      Cell_calculate_set_model_routine_name(dialog->calculate,button_label);
      Cell_calculate_set_dso_name(dialog->calculate,(char *)NULL);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"model_routine_menu_cb.  "
      "Missing cell calculate dialog");
  }
  LEAVE;
} /* modle_routine_menu_cb() */

static int setup_model_routine_chooser_widgets(
  struct Cell_calculate_dialog *dialog)
/*******************************************************************************
LAST MODIFIED : 18 December 2000

DESCRIPTION :
Sets up the model routine choosing widgets.
==============================================================================*/
{
  int return_code = 0;
  int i,menu_item_number;
  Widget widget,menu_item;
  char *routine_string = (char *)NULL;
  char *dso_string = (char *)NULL;

  ENTER(setup_model_routine_chooser_widgets);
  if (dialog && dialog->model_routine_pulldown &&
    dialog->model_routine_textfield && dialog->model_dso_textfield)
  {
    if (routine_string =
      Cell_calculate_get_model_routine_name(dialog->calculate))
    {
      dso_string = Cell_calculate_get_dso_name(dialog->calculate);
    }
    /* add the menu push buttons */
    menu_item = (Widget)NULL;
    for (i=0;i<number_of_built_in_models;i++)
    {
      widget = XtVaCreateManagedWidget("model_routine_chooser",
        xmPushButtonWidgetClass,dialog->model_routine_pulldown,
        XmNlabelString,XmStringCreateSimple(built_in_models_routines[i]),
        NULL);
      /* Check to see if the model specified is one of the built-in models */
      if (routine_string)
      {
        if (strcmp(built_in_models_routines[i],routine_string) == 0)
        {
          menu_item_number = i;
          menu_item = widget;
        }
      }
      /* add the callback */
      XtAddCallback(widget,XmNactivateCallback,model_routine_menu_cb,
        (XtPointer)dialog);
    }
    /* and add "User Defined" to the end of the list */
    widget = XtVaCreateManagedWidget("model_routine_chooser",
      xmPushButtonWidgetClass,dialog->model_routine_pulldown,
      XmNlabelString,XmStringCreateSimple(USER_DEFINED_MODEL_STRING),
      NULL);
    /* add the callback */
    XtAddCallback(widget,XmNactivateCallback,model_routine_menu_cb,
      (XtPointer)dialog);
    
    if (dso_string)
    {
      /* If a DSO is specified, then we have a user defined model */
      XtVaSetValues(dialog->model_routine_textfield,
        XmNvalue,routine_string,
        XmNsensitive,True,
        XmNeditable,True,
        NULL);
      XtVaSetValues(dialog->model_dso_textfield,
        XmNvalue,dso_string,
        XmNsensitive,True,
        XmNeditable,True,
        NULL);
      /* Set the option menu */
      XtVaSetValues(dialog->model_routine_option_menu,
        XmNmenuHistory,widget,
        NULL);
      return_code = 1;
    }
    else if (menu_item)
    {
      /* If we have found the routine name in the list of built-in models.. */
      XtVaSetValues(dialog->model_routine_textfield,
        XmNvalue,built_in_models_routines[menu_item_number],
        XmNsensitive,False,
        XmNeditable,False,
        NULL);
      XtVaSetValues(dialog->model_dso_textfield,
        XmNvalue,"<N/A>",
        XmNsensitive,False,
        XmNeditable,False,
        NULL);
      /* Set the option menu */
      XtVaSetValues(dialog->model_routine_option_menu,
        XmNmenuHistory,menu_item,
        NULL);
      /* Make sure that the routine name is set appropriately */
      Cell_calculate_set_model_routine_name(dialog->calculate,
        built_in_models_routines[menu_item_number]);
      Cell_calculate_set_dso_name(dialog->calculate,(char *)NULL);
      return_code = 1;
    }
    else
    {
      /* A user defined model routine is specified, but no DSO!! */
      XtVaSetValues(dialog->model_routine_textfield,
        XmNvalue,routine_string,
        XmNsensitive,True,
        XmNeditable,True,
        NULL);
      XtVaSetValues(dialog->model_dso_textfield,
        XmNvalue,dso_string,
        XmNsensitive,True,
        XmNeditable,True,
        NULL);
      /* Set the option menu */
      XtVaSetValues(dialog->model_routine_option_menu,
        XmNmenuHistory,widget,
        NULL);
      display_message(WARNING_MESSAGE,"setup_model_routine_chooser_widgets.  "
        "Need to specifiy a model DSO for user defined models");
      return_code = 1;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"setup_model_routine_chooser_widgets.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* setup_model_routine_chooser_widgets() */

static void intg_routine_changed_cb(Widget widget,XtPointer dialog_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
Callback for when the integrator routine name widget changes.
==============================================================================*/
{
  struct Cell_calculate_dialog *dialog;
  char *value_string,*widget_value_string;

  ENTER(intg_routine_changed_cb);
  USE_PARAMETER(call_data);
  if (dialog = (struct Cell_calculate_dialog *)dialog_void)
  {
    /* Get the value string from the widget */
    if (widget_value_string = XmTextFieldGetString(widget))
    {
      /* Check the widget string for a valid value and update the value */
      if (!Cell_calculate_set_intg_routine_name(dialog->calculate,
        widget_value_string))
      {
        /* The modified value is not valid, so redisplay the current value */
        if (value_string = Cell_calculate_get_intg_routine_name(
          dialog->calculate))
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
    display_message(ERROR_MESSAGE,"intg_routine_changed_cb.  "
      "Invalid argument(s)");
  }
  LEAVE;
} /* intg_routine_changed_cb() */

static void intg_dso_changed_cb(Widget widget,XtPointer dialog_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
Callback for when the integrator DSO name widget changes.
==============================================================================*/
{
  struct Cell_calculate_dialog *dialog;
  char *value_string,*widget_value_string;

  ENTER(intg_dso_changed_cb);
  USE_PARAMETER(call_data);
  if (dialog = (struct Cell_calculate_dialog *)dialog_void)
  {
    /* Get the value string from the widget */
    if (widget_value_string = XmTextFieldGetString(widget))
    {
      /* Check the widget string for a valid value and update the value */
      if (!Cell_calculate_set_intg_dso_name(dialog->calculate,
        widget_value_string))
      {
        /* The modified value is not valid, so redisplay the current value */
        if (value_string = Cell_calculate_get_intg_dso_name(
          dialog->calculate))
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
    display_message(ERROR_MESSAGE,"intg_dso_changed_cb.  "
      "Invalid argument(s)");
  }
  LEAVE;
} /* intg_dso_changed_cb() */

static void intg_routine_menu_cb(Widget widget,XtPointer dialog_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
Callback for the push buttons in the integrator routine option menu.
==============================================================================*/
{
  struct Cell_calculate_dialog *dialog;
  XmString str;
  char *button_label;

  ENTER(intg_routine_menu_cb);
  USE_PARAMETER(call_data);
  if (dialog = (struct Cell_calculate_dialog *)dialog_void)
  {
    USE_PARAMETER(dialog);
    XtVaGetValues(widget,
      XmNlabelString,&str,
      NULL);
    XmStringGetLtoR(str,XmSTRING_DEFAULT_CHARSET,&button_label);
    XmStringFree(str);
    if (strcmp(button_label,USER_DEFINED_INTG_STRING) == 0)
    {
      /* Make the text fields editable */
      XtVaSetValues(dialog->intg_routine_textfield,
        XmNsensitive,True,
        XmNeditable,True,
        XmNvalue,"",
        NULL);
      XtVaSetValues(dialog->intg_dso_textfield,
        XmNsensitive,True,
        XmNeditable,True,
        XmNvalue,"",
        NULL);
    }
    else
    {
      /* Make the text fields un-editable */
      XtVaSetValues(dialog->intg_routine_textfield,
        XmNvalue,button_label,
        XmNsensitive,False,
        XmNeditable,False,
        NULL);
      XtVaSetValues(dialog->intg_dso_textfield,
        XmNvalue,"<N/A>",
        XmNsensitive,False,
        XmNeditable,False,
        NULL);
      /* and set the model strings */
      Cell_calculate_set_intg_routine_name(dialog->calculate,button_label);
      Cell_calculate_set_intg_dso_name(dialog->calculate,(char *)NULL);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"intg_routine_menu_cb.  "
      "Missing cell calculate dialog");
  }
  LEAVE;
} /* intg_routine_menu_cb() */

static int setup_intg_routine_chooser_widgets(
  struct Cell_calculate_dialog *dialog)
/*******************************************************************************
LAST MODIFIED : 21 February 2001

DESCRIPTION :
Sets up the model routine choosing widgets.
==============================================================================*/
{
  int return_code = 0;
  int i,menu_item_number;
  Widget widget,menu_item;
  char *routine_string = (char *)NULL;
  char *dso_string = (char *)NULL;

  ENTER(setup_intg_routine_chooser_widgets);
  if (dialog && dialog->intg_routine_pulldown &&
    dialog->intg_routine_textfield && dialog->intg_dso_textfield)
  {
    if (routine_string =
      Cell_calculate_get_intg_routine_name(dialog->calculate))
    {
      dso_string = Cell_calculate_get_intg_dso_name(dialog->calculate);
    }
    /* add the menu push buttons */
    menu_item = (Widget)NULL;
    for (i=0;i<number_of_built_in_intgs;i++)
    {
      widget = XtVaCreateManagedWidget("intg_routine_chooser",
        xmPushButtonWidgetClass,dialog->intg_routine_pulldown,
        XmNlabelString,XmStringCreateSimple(built_in_intgs_routines[i]),
        NULL);
      /* Check to see if the integrator specified is one of the built-in
         models */
      if (routine_string)
      {
        if (strcmp(built_in_intgs_routines[i],routine_string) == 0)
        {
          menu_item_number = i;
          menu_item = widget;
        }
      }
      /* add the callback */
      XtAddCallback(widget,XmNactivateCallback,intg_routine_menu_cb,
        (XtPointer)dialog);
    }
    /* and add "User Defined" to the end of the list */
    widget = XtVaCreateManagedWidget("intg_routine_chooser",
      xmPushButtonWidgetClass,dialog->intg_routine_pulldown,
      XmNlabelString,XmStringCreateSimple(USER_DEFINED_INTG_STRING),
      NULL);
    /* add the callback */
    XtAddCallback(widget,XmNactivateCallback,intg_routine_menu_cb,
      (XtPointer)dialog);
    
    if (dso_string)
    {
      /* If a DSO is specified, then we have a user defined model */
      XtVaSetValues(dialog->intg_routine_textfield,
        XmNvalue,routine_string,
        XmNsensitive,True,
        XmNeditable,True,
        NULL);
      XtVaSetValues(dialog->intg_dso_textfield,
        XmNvalue,dso_string,
        XmNsensitive,True,
        XmNeditable,True,
        NULL);
      /* Set the option menu */
      XtVaSetValues(dialog->intg_routine_option_menu,
        XmNmenuHistory,widget,
        NULL);
      return_code = 1;
    }
    else if (menu_item)
    {
      /* If we have found the routine name in the list of built-in models.. */
      XtVaSetValues(dialog->intg_routine_textfield,
        XmNvalue,built_in_intgs_routines[menu_item_number],
        XmNsensitive,False,
        XmNeditable,False,
        NULL);
      XtVaSetValues(dialog->intg_dso_textfield,
        XmNvalue,"<N/A>",
        XmNsensitive,False,
        XmNeditable,False,
        NULL);
      /* Set the option menu */
      XtVaSetValues(dialog->intg_routine_option_menu,
        XmNmenuHistory,menu_item,
        NULL);
      /* Make sure that the routine name is set appropriately */
      Cell_calculate_set_intg_routine_name(dialog->calculate,
        built_in_intgs_routines[menu_item_number]);
      Cell_calculate_set_intg_dso_name(dialog->calculate,(char *)NULL);
      return_code = 1;
    }
    else
    {
      /* A user defined integrator routine is specified, but no DSO!! */
      XtVaSetValues(dialog->intg_routine_textfield,
        XmNvalue,routine_string,
        XmNsensitive,True,
        XmNeditable,True,
        NULL);
      XtVaSetValues(dialog->intg_dso_textfield,
        XmNvalue,dso_string,
        XmNsensitive,True,
        XmNeditable,True,
        NULL);
      /* Set the option menu */
      XtVaSetValues(dialog->intg_routine_option_menu,
        XmNmenuHistory,widget,
        NULL);
      display_message(WARNING_MESSAGE,"setup_intg_routine_chooser_widgets.  "
        "Need to specifiy a integrator DSO for user defined integrators");
      return_code = 1;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"setup_intg_routine_chooser_widgets.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* setup_intg_routine_chooser_widgets() */

/*
Global functions
----------------
*/
struct Cell_calculate_dialog *CREATE(Cell_calculate_dialog)(
  struct Cell_calculate *cell_calculate,Widget parent,
  struct Cell_interface *cell_interface,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 18 December 2000

DESCRIPTION :
Creates the calculation dialog
==============================================================================*/
{
  struct Cell_calculate_dialog *cell_calculate_dialog;
  Atom WM_DELETE_WINDOW;
  MrmType cell_calculate_dialog_class;
  static MrmRegisterArg callback_list[] = {
    {"tstart_changed_callback",(XtPointer)tstart_changed_cb},
    {"tend_changed_callback",(XtPointer)tend_changed_cb},
    {"dt_changed_callback",(XtPointer)dt_changed_cb},
    {"tabt_changed_callback",(XtPointer)tabt_changed_cb},
    {"calculate_button_callback",(XtPointer)calculate_button_cb},
    {"close_button_callback",(XtPointer)close_button_cb},
    {"help_button_callback",(XtPointer)help_button_cb},
    {"model_routine_changed_cb",(XtPointer)model_routine_changed_cb},
    {"model_dso_changed_cb",(XtPointer)model_dso_changed_cb},
    {"intg_routine_changed_cb",(XtPointer)intg_routine_changed_cb},
    {"intg_dso_changed_cb",(XtPointer)intg_dso_changed_cb},
    {"id_model_routine_pulldown",
     (XtPointer)DIALOG_IDENTIFY(cell_calculate_dialog,model_routine_pulldown)},
    {"id_model_routine_textfield",
     (XtPointer)DIALOG_IDENTIFY(cell_calculate_dialog,model_routine_textfield)},
    {"id_model_dso_textfield",
     (XtPointer)DIALOG_IDENTIFY(cell_calculate_dialog,model_dso_textfield)},
    {"id_model_option_menu",(XtPointer)
     DIALOG_IDENTIFY(cell_calculate_dialog,model_routine_option_menu)},
    {"id_intg_routine_pulldown",
     (XtPointer)DIALOG_IDENTIFY(cell_calculate_dialog,intg_routine_pulldown)},
    {"id_intg_routine_textfield",
     (XtPointer)DIALOG_IDENTIFY(cell_calculate_dialog,intg_routine_textfield)},
    {"id_intg_dso_textfield",
     (XtPointer)DIALOG_IDENTIFY(cell_calculate_dialog,intg_dso_textfield)},
    {"id_intg_option_menu",(XtPointer)
     DIALOG_IDENTIFY(cell_calculate_dialog,intg_routine_option_menu)}
  }; /* callback_list */
  static MrmRegisterArg identifier_list[] = {
    {"calculate_dialog",(XtPointer)NULL},
    {"tstart_value_string",(XtPointer)NULL},
    {"tend_value_string",(XtPointer)NULL},
    {"dt_value_string",(XtPointer)NULL},
    {"tabt_value_string",(XtPointer)NULL}
  }; /* identifier_list */
#define XmNnameColour "nameColour"
#define XmCNameColour "NameColour"
#define XmNvalueColour "valueColour"
#define XmCValueColour "ValueColour"
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
      XmNlabelColour,
      XmCLabelColour,
      XmRPixel,
      sizeof(Pixel),
      XtOffsetOf(User_settings,label_colour),
      XmRString,
      "blue"
    }
  }; /* resources */
  
  ENTER(CREATE(Cell_calculate_dialog));
  if (cell_calculate && parent && user_interface)
  {
    if (MrmOpenHierarchy_base64_string(cell_calculate_dialog_uidh,
      &cell_calculate_dialog_hierarchy,
      &cell_calculate_dialog_hierarchy_open))
    {
      if (ALLOCATE(cell_calculate_dialog,struct Cell_calculate_dialog,1))
      {
        /* Initialise the structure */
        cell_calculate_dialog->calculate = cell_calculate;
        cell_calculate_dialog->interface = cell_interface;
        cell_calculate_dialog->user_interface = user_interface;
        cell_calculate_dialog->shell = (Widget)NULL;
        cell_calculate_dialog->window = (Widget)NULL;
        /* Make the dialog shell */
        if (cell_calculate_dialog->shell =
          XtVaCreatePopupShell("cell_calculate_dialog_shell",
            xmDialogShellWidgetClass,parent,
            XmNmwmDecorations,MWM_DECOR_ALL,
            XmNmwmFunctions,MWM_FUNC_ALL,
            XmNdeleteResponse,XmDO_NOTHING,
            XmNtransient,FALSE,
            XmNtitle,"Cell Calculate",
            NULL))
        {
          /* Identify the shell for the busy icon */
          create_Shell_list_item(&(cell_calculate_dialog->shell),
            user_interface);
          /* Add the destroy callback */
          XtAddCallback(cell_calculate_dialog->shell,XmNdestroyCallback,
            close_button_cb,
            (XtPointer)cell_calculate_dialog);
          WM_DELETE_WINDOW=XmInternAtom(
            XtDisplay(cell_calculate_dialog->shell),
            "WM_DELETE_WINDOW",FALSE);
          XmAddWMProtocolCallback(cell_calculate_dialog->shell,
            WM_DELETE_WINDOW,close_button_cb,
            (XtPointer)cell_calculate_dialog);
          /* register cellbacks in UIL */
          if (MrmSUCCESS == MrmRegisterNamesInHierarchy(
            cell_calculate_dialog_hierarchy,callback_list,
            XtNumber(callback_list)))
          {
            if (ALLOCATE(cell_calculate_dialog->user_settings,
              User_settings,1))
            {
              /* the pointer to this dialog */
              identifier_list[0].value =
                (XtPointer)cell_calculate_dialog;
              /* the current start time */
              identifier_list[1].value =
                (XtPointer)Cell_calculate_get_start_time_as_string(
                  cell_calculate);
              /* the current end time */
              identifier_list[2].value =
                (XtPointer)Cell_calculate_get_end_time_as_string(
                  cell_calculate);
              /* the current dt */
              identifier_list[3].value =
                (XtPointer)Cell_calculate_get_dt_as_string(cell_calculate);
              /* the current tabt */
              identifier_list[4].value =
                (XtPointer)Cell_calculate_get_tabt_as_string(cell_calculate);
              /* Register the identifiers */
              if (MrmSUCCESS ==
                MrmRegisterNamesInHierarchy(
                  cell_calculate_dialog_hierarchy,
                  identifier_list,XtNumber(identifier_list)))
              {
                /* Fetch the window widget */
                if (MrmSUCCESS == MrmFetchWidget(
                  cell_calculate_dialog_hierarchy,
                  "calculate_dialog_widget",
                  cell_calculate_dialog->shell,
                  &(cell_calculate_dialog->window),
                  &cell_calculate_dialog_class))
                {
                  /* Retrieve settings */
                  XtVaGetApplicationResources(
                    cell_calculate_dialog->window,
                    (XtPointer)(cell_calculate_dialog->user_settings),
                    resources,XtNumber(resources),NULL);
                  /* Set-up the model routine chooser menu */
                  setup_model_routine_chooser_widgets(cell_calculate_dialog);
                  /* Set-up the integrator routine chooser menu */
                  setup_intg_routine_chooser_widgets(cell_calculate_dialog);
                  XtManageChild(cell_calculate_dialog->window);
                  XtRealizeWidget(cell_calculate_dialog->shell);
                }
                else
                {
                  display_message(ERROR_MESSAGE,
                    "CREATE(Cell_calculate_dialog). "
                    "Unable to fetch the window widget");
                  DESTROY(Cell_calculate_dialog)(
                    &cell_calculate_dialog);
                  cell_calculate_dialog =
                    (struct Cell_calculate_dialog *)NULL;
                }
              }
              else
              {
                display_message(ERROR_MESSAGE,
                  "CREATE(Cell_calculate_dialog). "
                  "Unable to register the identifiers");
                DESTROY(Cell_calculate_dialog)(
                  &cell_calculate_dialog);
                cell_calculate_dialog =
                  (struct Cell_calculate_dialog *)NULL;
              }
            }
            else
            {
              display_message(ERROR_MESSAGE,
                "CREATE(Cell_calculate_dialog). "
                "Unable to allocate memory for the user settings");
              DESTROY(Cell_calculate_dialog)(
                &cell_calculate_dialog);
              cell_calculate_dialog =
                (struct Cell_calculate_dialog *)NULL;
            }
          }
          else
          {
            display_message(ERROR_MESSAGE,
              "CREATE(Cell_calculate_dialog). "
              "Unable to register the callbacks");
            DESTROY(Cell_calculate_dialog)(
              &cell_calculate_dialog);
            cell_calculate_dialog =
              (struct Cell_calculate_dialog *)NULL;
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"CREATE(Cell_calculate_dialog). "
            "Unable to create the dialog shell");
          DESTROY(Cell_calculate_dialog)(&cell_calculate_dialog);
          cell_calculate_dialog =
            (struct Cell_calculate_dialog *)NULL;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"CREATE(Cell_calculate_dialog). "
          "Unable to allocate memory for the dialog");
        cell_calculate_dialog =
          (struct Cell_calculate_dialog *)NULL;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"CREATE(Cell_calculate_dialog). "
        "Unable to open the dialog hierarchy");
      cell_calculate_dialog =
        (struct Cell_calculate_dialog *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Cell_calculate_dialog). "
      "Invalid argument(s)");
    cell_calculate_dialog = (struct Cell_calculate_dialog *)NULL;
  }
  LEAVE;
  return(cell_calculate_dialog);
} /* CREATE(Cell_calculate_dialog)() */

int DESTROY(Cell_calculate_dialog)(
  struct Cell_calculate_dialog **dialog_address)
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Destroys the Cell_calculate_dialog object.
==============================================================================*/
{
  int return_code;
  struct Cell_calculate_dialog *dialog;
  
  ENTER(DESTROY(Cell_calculate_dialog));
  if (dialog_address && (dialog = *dialog_address))
	{
    /* make sure the window isn't currently being displayed */
    if (dialog->shell)
    {
      XtPopdown(dialog->shell);
      /* remove the dialog from the shell list */
      destroy_Shell_list_item_from_shell(&(dialog->shell),
        dialog->user_interface);
      /* destroying the shell should destroy all the children */
      /* But this causes problems when it calls the close_button_cb() so I'll
       * take it out for now!!
       */
      /*XtDestroyWidget(dialog->shell);
        dialog->shell = (Widget)NULL;*/
    }
		DEALLOCATE(*dialog_address);
    *dialog_address = (struct Cell_calculate_dialog *)NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Cell_calculate_dialog).  Invalid argument(s)");
		return_code=0;
	}
  LEAVE;
  return(return_code);
} /* DESTROY(Cell_calculate_dialog)() */

int Cell_calculate_dialog_pop_up(struct Cell_calculate_dialog *dialog)
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Pops up the calculate dialog
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_calculate_dialog_pop_up);
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
    display_message(ERROR_MESSAGE,"Cell_calculate_dialog_pop_up.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_calculate_dialog_pop_up() */

int Cell_calculate_dialog_pop_down(struct Cell_calculate_dialog *dialog)
/*******************************************************************************
LAST MODIFIED : 22 January 2001

DESCRIPTION :
Pops down the calculate dialog
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_calculate_dialog_pop_down);
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
    display_message(ERROR_MESSAGE,"Cell_calculate_dialog_pop_down.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_calculate_dialog_pop_down() */

