/*******************************************************************************
FILE : variables_dialog.c

LAST MODIFIED : 16 September 1999

DESCRIPTION :
Functions and structures for using the variables dialog box.
==============================================================================*/
#include <stdio.h>
#if defined (MOTIF)
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Xm/MwmUtil.h>
#include <Xm/DialogS.h>
#include <Xm/Label.h>
#include <Xm/Protocols.h>
#endif /* if defined (MOTIF) */
#include "user_interface/user_interface.h"
#include "general/debug.h"
#include "cell/cell_variable.h"
#include "cell/variables_dialog.h"
#include "cell/variables_dialog.uid64"
#include "cell/cell_window.h"

/*
Module types
============
*/
typedef struct Variables_dialog_user_settings User_settings;

/*
Module variables
================
*/
#if defined (MOTIF)
static int variables_dialog_hierarchy_open=0;
static MrmHierarchy variables_dialog_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
================
*/
static void identify_variables_rowcol(Widget widget,
  XtPointer cell_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 07 February 1999

DESCRIPTION :
ID's the variables row column manager widget
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(idetify_variables_rowcol);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    cell->variables_dialog->rowcol = widget;
  }
  else
  {
    display_message(ERROR_MESSAGE,"identify_variables_rowcol. "
      "Missing Cell window");
  }
  LEAVE;
} /* END identify_variables_rowcol() */

static void ok_button_callback(Widget widget,
  XtPointer cell_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 05 February 1999

DESCRIPTION :
Callback for the OK button in the variables dialog.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(ok_button_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (cell->variables_dialog && cell->variables_dialog->shell)
    {
      XtPopdown(cell->variables_dialog->shell);
      /* update the value fields */
      if (cell->variables)
      {
        update_variable_values(cell->variables);
        reset_variable_values(cell->variables);
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"ok_button_callback. "
        "Missing variables dialog");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"ok_button_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END ok_button_callback() */

static void reset_button_callback(Widget widget,
  XtPointer cell_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 05 February 1999

DESCRIPTION :
Callback for the RESET button
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(reset_button_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    reset_variable_values(cell->variables);
  }
  else
  {
    display_message(ERROR_MESSAGE,"reset_button_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END reset_button_callback() */

static void cancel_button_callback(Widget widget,
  XtPointer cell_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 05 February 1999

DESCRIPTION :
Callback for the CANCEL button
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(cancel_button_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (cell->variables_dialog && cell->variables_dialog->shell)
    {
      XtPopdown(cell->variables_dialog->shell);
      reset_variable_values(cell->variables);
    }
    else
    {
      display_message(ERROR_MESSAGE,"cancel_button_callback. "
        "Missing variables dialog");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"cancel_button_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END cancel_button_callback() */

static void help_button_callback(Widget widget,
  XtPointer cell_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 February 1999

DESCRIPTION :
Callback for the HELP button
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(help_button_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  USE_PARAMETER(cell);
  if (cell = (struct Cell_window *)cell_window)
  {
    display_message(INFORMATION_MESSAGE,"help_button_callback. "
      "sorry, no help available for the variables dialog!!!\n");
  }
  else
  {
    display_message(ERROR_MESSAGE,"help_button_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END help_button_callback() */

static void variables_dialog_destroy_callback(Widget widget,
  XtPointer cell_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 05 February 1999

DESCRIPTION :
Callback for when the variables dialog is destroy via the window manager menu ??
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(variables_dialog_destroy_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (cell->variables_dialog)
    {
      if (cell->variables_dialog->shell)
      {
        /* remove the shell from the shell list */
        destroy_Shell_list_item_from_shell(&(cell->variables_dialog->shell),
          cell->user_interface);
        /* make sure the dialog is no longer visible */
        XtPopdown(cell->variables_dialog->shell);
        /* Unmanage the shell */
        XtUnmanageChild(cell->variables_dialog->shell);
        /*display_message(INFORMATION_MESSAGE,
          "variables_dialog_destroy_callback. "
          "Unmanage variables_dialog->shell\n");*/
      }
      DEALLOCATE(cell->variables_dialog);
      cell->variables_dialog = (struct Variables_dialog *)NULL;
      /*display_message(INFORMATION_MESSAGE,
        "variables_dialog_destroy_callback. "
        "DEALLOCATE variables_dialog\n");*/
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"variables_dialog_destroy_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END variables_dialog_destroy_callback() */

static int create_variables_dialog(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 26 August 1999

DESCRIPTION :
Create a new variables dialog.
==============================================================================*/
{
  int return_code = 0;
  Atom WM_DELETE_WINDOW;
  MrmType variables_dialog_class;
  static MrmRegisterArg callback_list[] = {
    {"identify_variables_rowcol",(XtPointer)identify_variables_rowcol},
    {"ok_button_callback",(XtPointer)ok_button_callback},
    {"reset_button_callback",(XtPointer)reset_button_callback},
    {"cancel_button_callback",(XtPointer)cancel_button_callback},
    {"help_button_callback",(XtPointer)help_button_callback}
  }; /* callback_list */
  static MrmRegisterArg identifier_list[] = {
    {"cell_window_structure",(XtPointer)NULL},
    {"window_width",(XtPointer)NULL},
    {"window_height",(XtPointer)NULL}    
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
  
  ENTER(create_variables_dialog);
  if (cell != (struct Cell_window *)NULL)
  {
    if (cell->variables_dialog != (struct Variables_dialog *)NULL)
    {
      /* destroy any existing variables_dialog */
      variables_dialog_destroy_callback((Widget)NULL,(XtPointer)cell,
        (XtPointer)NULL);
    }
    if (MrmOpenHierarchy_base64_string(variables_dialog_uid64,
      &variables_dialog_hierarchy,&variables_dialog_hierarchy_open))
    {
      if (ALLOCATE(cell->variables_dialog,struct Variables_dialog,1))
      {
        /* initialise the structure */
        cell->variables_dialog->shell = (Widget)NULL;
        cell->variables_dialog->window = (Widget)NULL;
        cell->variables_dialog->rowcol = (Widget)NULL;
        /* make the dialog shell */
        if (cell->variables_dialog->shell =
          XtVaCreatePopupShell("variables_dialog_shell",
            xmDialogShellWidgetClass,cell->shell,
            XmNmwmDecorations,MWM_DECOR_ALL,
            XmNmwmFunctions,MWM_FUNC_ALL,
            XmNdeleteResponse,XmDESTROY,
            XmNtransient,FALSE,
            NULL))
        {
          /* identify the shell for the busy icon */
          create_Shell_list_item(&(cell->variables_dialog->shell),
            cell->user_interface);
          /* add the destroy callback */
          XtAddCallback(cell->variables_dialog->shell,XmNdestroyCallback,
            variables_dialog_destroy_callback,(XtPointer)cell);
          WM_DELETE_WINDOW=XmInternAtom(
            XtDisplay(cell->variables_dialog->shell),
            "WM_DELETE_WINDOW",FALSE);
          XmAddWMProtocolCallback(cell->variables_dialog->shell,
            WM_DELETE_WINDOW,variables_dialog_destroy_callback,(XtPointer)cell);
          /* register cellbacks in UIL */
          if (MrmSUCCESS == MrmRegisterNamesInHierarchy(
            variables_dialog_hierarchy,callback_list,XtNumber(callback_list)))
          {
            /* set the identifier's values */
            width = cell->user_interface->screen_width/2;
            height = cell->user_interface->screen_height/2;
            identifier_list[0].value = (XtPointer)cell;
            identifier_list[1].value = (XtPointer)width;
            identifier_list[2].value = (XtPointer)height;
            /* register the identifiers */
            if (MrmSUCCESS ==
              MrmRegisterNamesInHierarchy(variables_dialog_hierarchy,
                identifier_list,XtNumber(identifier_list)))
            {
              /* fetch the window widget */
              if (MrmSUCCESS == MrmFetchWidget(variables_dialog_hierarchy,
                "variables_dialog",cell->variables_dialog->shell,
                &(cell->variables_dialog->window),&variables_dialog_class))
              {
                /* retrieve settings */
                if (ALLOCATE(user_settings,User_settings,1))
                {
                  XtVaGetApplicationResources(cell->variables_dialog->window,
                    (XtPointer)user_settings,resources,XtNumber(resources),
                    NULL);
                  if (cell->variables != (struct Cell_variable *)NULL)
                  {
                    if (add_variables_to_variables_dialog(cell,user_settings))
                    {
                      XtManageChild(cell->variables_dialog->window);
                      XtRealizeWidget(cell->variables_dialog->shell);
                      format_variable_widgets(cell->variables);
                      return_code = 1;
                    }
                    else
                    {
                      display_message(ERROR_MESSAGE,"create_variables_dialog. "
                        "Unable to add variables to the dialog");
                      DEALLOCATE(cell->variables_dialog);
                      cell->variables_dialog = (struct Variables_dialog *)NULL;
                      return_code = 0;                    
                    }
                  }
                  else
                  {
                    /*display_message(INFORMATION_MESSAGE,
                      "create_variables_dialog. "
                      "No variables to display\n");*/
                    XtManageChild(cell->variables_dialog->window);
                    XtRealizeWidget(cell->variables_dialog->shell);
                    return_code = 1;
                  }
                }
                else
                {
                  display_message(ERROR_MESSAGE,"create_variables_dialog. "
                    "Unable to allocate memory for the user settings");
                  DEALLOCATE(cell->variables_dialog);
                  cell->variables_dialog = (struct Variables_dialog *)NULL;
                  return_code = 0;
                }
              }
              else
              {
                display_message(ERROR_MESSAGE,"create_variables_dialog. "
                  "Unable to fetch the window widget");
                DEALLOCATE(cell->variables_dialog);
                cell->variables_dialog = (struct Variables_dialog *)NULL;
                return_code = 0;
              }
            }
            else
            {
              display_message(ERROR_MESSAGE,"create_variables_dialog. "
                "Unable to register the identifiers");
              DEALLOCATE(cell->variables_dialog);
              cell->variables_dialog = (struct Variables_dialog *)NULL;
              return_code = 0;  
            }
          }
          else
          {
            display_message(ERROR_MESSAGE,"create_variables_dialog. "
              "Unable to register the callbacks");
            DEALLOCATE(cell->variables_dialog);
            cell->variables_dialog = (struct Variables_dialog *)NULL;
            return_code = 0;  
          }
        }
        else
        {
        display_message(ERROR_MESSAGE,"create_variables_dialog. "
          "Unable to create the dialog shell for the variables dialog");
        DEALLOCATE(cell->variables_dialog);
        cell->variables_dialog = (struct Variables_dialog *)NULL;
        return_code = 0;          
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"create_variables_dialog. "
          "Unable to allocate memory for the model dialog");
        cell->variables_dialog = (struct Variables_dialog *)NULL;
        return_code = 0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"create_variables_dialog. "
        "Unable to open the variables dialog hierarchy");
      cell->variables_dialog = (struct Variables_dialog *)NULL;
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"create_variables_dialog. "
      "Missing Cell window");
    cell->variables_dialog = (struct Variables_dialog *)NULL;
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END create_variables_dialog() */

/*
Global functions
================
*/
int bring_up_variables_dialog(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 05 February 1999

DESCRIPTION :
If there is a variables dialog in existence, then bring it to the front, else
create a new one.
==============================================================================*/
{
  int return_code = 0;
  
  ENTER(bring_up_variables_dialog);
  if (cell != (struct Cell_window *)NULL)
  {
    if (cell->variables_dialog == (struct Variables_dialog *)NULL)
    {
      /*display_message(INFORMATION_MESSAGE,"bring_up_variables_dialog. "
        "creating new dialog\n");*/
      /* model dialog does not exist, so create a new one */
      if (create_variables_dialog(cell))
      {
        XtPopup(cell->variables_dialog->shell,XtGrabNone);
        return_code = 1;
      }
      else
      {
        display_message(ERROR_MESSAGE,"bring_up_variables_dialog. "
          "Error creating the model dialog");
        cell->variables_dialog = (struct Variables_dialog *)NULL;
        return_code = 0;
      }
    }
    else
    {
      /*display_message(INFORMATION_MESSAGE,"bring_up_variables_dialog. "
        "dialog exists\n");*/
      /* dialog already exists so just pop it up */
      XtPopup(cell->variables_dialog->shell,XtGrabNone);
      return_code = 1;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"bring_up_variables_dialog. "
      "Missing Cell window");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END bring_up_variables_dialog() */

void close_variables_dialog(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 07 February 1999

DESCRIPTION :
If there is a variables dialog in existence, then destroy it. Used when
variables arfe read from file to force the dialog to be re-created.
==============================================================================*/
{
  ENTER(close_variables_dialog);
  /*display_message(INFORMATION_MESSAGE,"close_variables_dialog.\n");*/
  variables_dialog_destroy_callback((Widget)NULL,(XtPointer)cell,
    (XtPointer)NULL);
  LEAVE;
} /* END close_variables_dialog() */
