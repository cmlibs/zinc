/*******************************************************************************
FILE : distributed_editing_dialog.c

LAST MODIFIED : 01 February 2001

DESCRIPTION :
The dialog box for distributed parameter editing. Only used when
CELL_DISTRIBUTED is defined.
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

#if defined (CELL_DISTRIBUTED)

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

#include "cell/distributed_editing_dialog.uidh"
#include "cell/distributed_editing_dialog.h"
#include "user_interface/gui_dialog_macros.h"

/*
Module types
============
*/

/*
Module variables
================
*/
#if defined (MOTIF)
static int distributed_editing_dialog_hierarchy_open=0;
static MrmHierarchy distributed_editing_dialog_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
================
*/
DECLARE_DIALOG_IDENTIFY_FUNCTION(distributed_editing_dialog, \
  Distributed_editing_dialog,element_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(distributed_editing_dialog, \
  Distributed_editing_dialog,point_number_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(distributed_editing_dialog, \
  Distributed_editing_dialog,grid_field_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(distributed_editing_dialog, \
  Distributed_editing_dialog,grid_value_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(distributed_editing_dialog, \
  Distributed_editing_dialog,description_label)

static void close_distributed_editing_dialog(Widget widget,
  XtPointer distributed_editing_dialog_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 13 January 2001

DESCRIPTION :
Called when the Exit function is selected from the window manager menu in the
distributed editing dialog.
==============================================================================*/
{
  struct Distributed_editing_dialog *dialog;

  ENTER(close_distributed_editing_dialog);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
  if (dialog =
    (struct Distributed_editing_dialog *)distributed_editing_dialog_void)
  {
    if (dialog->shell)
    {
      XtPopdown(dialog->shell);
    }
    /* Make sure the activation toggle button gets turned off */
    if (dialog->activation)
    {
      /* Make sure the callbacks are processed */
      XmToggleButtonSetState(dialog->activation,False,True);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"close_distributed_editing_dialog. "
      "Missing distributed editing dialog");
  }
  LEAVE;
} /* close_distributed_editing_dialog() */

static void apply_button_CB(Widget widget,
  XtPointer distributed_editing_dialog_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 January 2001

DESCRIPTION :
Callback for the "apply" button in the distributed editing dialog. Puts the
values from the Cell variables into the current element point
==============================================================================*/
{
  struct Distributed_editing_dialog *dialog;

  ENTER(apply_button_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
  if (dialog =
    (struct Distributed_editing_dialog *)distributed_editing_dialog_void)
  {
    if (!Distributed_editing_interface_update_element_point(dialog->interface,
      /*apply all*/0))
    {
      display_message(ERROR_MESSAGE,"apply_button_CB.  "
        "Unable to set the element point values");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"apply_button_CB. "
      "Missing distributed editing dialog");
  }
  LEAVE;
} /* apply_button_CB() */

static void apply_all_button_CB(Widget widget,
  XtPointer distributed_editing_dialog_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 January 2001

DESCRIPTION :
Callback for the "apply all" button in the distributed editing dialog. Puts the
values from the Cell variables into all the currently selected element point
==============================================================================*/
{
  struct Distributed_editing_dialog *dialog;

  ENTER(apply_all_button_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
  if (dialog =
    (struct Distributed_editing_dialog *)distributed_editing_dialog_void)
  {
    if (!Distributed_editing_interface_update_element_point(dialog->interface,
      /*apply all*/1))
    {
      display_message(ERROR_MESSAGE,"apply_all_button_CB.  "
        "Unable to set the element point values");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"apply_all_button_CB. "
      "Missing distributed editing dialog");
  }
  LEAVE;
} /* apply_all_button_CB() */

static void reset_button_CB(Widget widget,
  XtPointer distributed_editing_dialog_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 22 January 2001

DESCRIPTION :
Callback for the "reset" button in the distributed editing dialog. Resets the
Cell variable values to the element point values.
==============================================================================*/
{
  struct Distributed_editing_dialog *dialog;

  ENTER(apply_button_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
  if (dialog =
    (struct Distributed_editing_dialog *)distributed_editing_dialog_void)
  {
    if (!Distributed_editing_interface_update_from_element_point(
      dialog->interface))
    {
      display_message(ERROR_MESSAGE,"reset_button_CB.  "
        "Unable to reset the Cell variable values");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"reset_button_CB. "
      "Missing distributed editing dialog");
  }
  LEAVE;
} /* reset_button_CB() */

/*
Global Functions
================
*/
struct Distributed_editing_dialog *CREATE(Distributed_editing_dialog)(
  struct Distributed_editing_interface *interface,Widget parent,
  struct User_interface *user_interface,Widget activation)
/*******************************************************************************
LAST MODIFIED : 13 January 2001

DESCRIPTION :
Creates a distributed editing dialog object. <activation> is the toggle button
widget which activated the dialog.
==============================================================================*/
{
  struct Distributed_editing_dialog *distributed_editing_dialog;
  Atom WM_DELETE_WINDOW;
  MrmType distributed_editing_dialog_class;
  static MrmRegisterArg callback_list[] = {
    {"id_element_form",
     (XtPointer)DIALOG_IDENTIFY(distributed_editing_dialog,element_form)},
    {"id_point_number_text",
     (XtPointer)DIALOG_IDENTIFY(distributed_editing_dialog,point_number_text)},
    {"id_grid_field_form",
     (XtPointer)DIALOG_IDENTIFY(distributed_editing_dialog,grid_field_form)},
    {"id_grid_value_text",
     (XtPointer)DIALOG_IDENTIFY(distributed_editing_dialog,grid_value_text)},
    {"id_description_label",
     (XtPointer)DIALOG_IDENTIFY(distributed_editing_dialog,description_label)},
    {"point_number_text_CB",
     (XtPointer)Distributed_editing_interface_point_number_text_CB},
    {"grid_value_text_CB",
     (XtPointer)Distributed_editing_interface_grid_value_text_CB},
    {"apply_button_CB",(XtPointer)apply_button_CB},
    {"apply_all_button_CB",apply_all_button_CB},
    {"reset_button_CB",reset_button_CB}
  }; /* callback_list */
  static MrmRegisterArg identifier_list[] = {
    {"distributed_editing_dialog",(XtPointer)NULL},
    {"distributed_editing_interface",(XtPointer)NULL}
  }; /* identifier_list */

  ENTER(CREATE(Distributed_editing_dialog));
  if (interface && parent && user_interface && activation)
  {
    if (MrmOpenHierarchy_base64_string(distributed_editing_dialog_uidh,
      &distributed_editing_dialog_hierarchy,
      &distributed_editing_dialog_hierarchy_open))
    {
      /* Allocate memory for the object */
      if (ALLOCATE(distributed_editing_dialog,
        struct Distributed_editing_dialog,1))
      {
        /* Initialise the object */
        distributed_editing_dialog->interface = interface;
        distributed_editing_dialog->user_interface = user_interface;
        distributed_editing_dialog->activation = activation;
        distributed_editing_dialog->shell = (Widget)NULL;
        distributed_editing_dialog->window = (Widget)NULL;
        distributed_editing_dialog->element_form = (Widget)NULL;
        distributed_editing_dialog->element_widget = (Widget)NULL;
        distributed_editing_dialog->point_number_text = (Widget)NULL;
        distributed_editing_dialog->grid_field_form = (Widget)NULL;
        distributed_editing_dialog->grid_field_widget = (Widget)NULL;
        distributed_editing_dialog->grid_value_text = (Widget)NULL;
        distributed_editing_dialog->description_label = (Widget)NULL;
        /* Make the dialog shell */
        if (distributed_editing_dialog->shell =
          XtVaCreatePopupShell("distributed_editing_dialog_shell",
            xmDialogShellWidgetClass,parent,
            XmNmwmDecorations,MWM_DECOR_ALL,
            XmNmwmFunctions,MWM_FUNC_ALL,
            XmNdeleteResponse,XmDO_NOTHING,
            XmNtransient,FALSE,
            NULL))
        {
          /* Identify the shell for the busy icon */
          create_Shell_list_item(&(distributed_editing_dialog->shell),
            user_interface);
          /* Add the destroy callback */
          XtAddCallback(distributed_editing_dialog->shell,XmNdestroyCallback,
            close_distributed_editing_dialog,
            (XtPointer)distributed_editing_dialog);
          WM_DELETE_WINDOW=XmInternAtom(
            XtDisplay(distributed_editing_dialog->shell),
            "WM_DELETE_WINDOW",FALSE);
          XmAddWMProtocolCallback(distributed_editing_dialog->shell,
            WM_DELETE_WINDOW,close_distributed_editing_dialog,
            (XtPointer)distributed_editing_dialog);
          /* register cellbacks in UIL */
          if (MrmSUCCESS == MrmRegisterNamesInHierarchy(
            distributed_editing_dialog_hierarchy,callback_list,
            XtNumber(callback_list)))
          {
            /* the pointer to this dialog */
            identifier_list[0].value =
              (XtPointer)distributed_editing_dialog;
            identifier_list[1].value =
              (XtPointer)interface;
            /* Register the identifiers */
            if (MrmSUCCESS ==
              MrmRegisterNamesInHierarchy(
                distributed_editing_dialog_hierarchy,
                identifier_list,XtNumber(identifier_list)))
            {
              /* Fetch the window widget */
              if (MrmSUCCESS == MrmFetchWidget(
                distributed_editing_dialog_hierarchy,
                "dist_editing_dialog_widget",
                distributed_editing_dialog->shell,
                &(distributed_editing_dialog->window),
                &distributed_editing_dialog_class))
              {
                /* Create the chooser widgets */
                if (Distributed_editing_interface_create_choosers(interface,
                  (void *)distributed_editing_dialog))
                {
                  XtManageChild(distributed_editing_dialog->window);
                  XtRealizeWidget(distributed_editing_dialog->shell);
                }
                else
                {
                  display_message(ERROR_MESSAGE,
                    "CREATE(Distributed_editing_dialog).  "
                    "Unable to create the chooser widgets");
                  DESTROY(Distributed_editing_dialog)(
                    &distributed_editing_dialog);
                }
              }
              else
              {
                display_message(ERROR_MESSAGE,
                  "CREATE(Distributed_editing_dialog).  "
                  "Unable to fetch the window widget");
                DESTROY(Distributed_editing_dialog)(
                  &distributed_editing_dialog);
              }
            }
            else
            {
              display_message(ERROR_MESSAGE,
                "CREATE(Distributed_editing_dialog).  "
                "Unable to register the identifiers");
              DESTROY(Distributed_editing_dialog)(
                &distributed_editing_dialog);
            }
          }
          else
          {
            display_message(ERROR_MESSAGE,
              "CREATE(Distributed_editing_dialog).  "
              "Unable to register the callbacks");
            DESTROY(Distributed_editing_dialog)(
              &distributed_editing_dialog);
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"CREATE(Distributed_editing_dialog).  "
            "Unable to create the dialog shell");
          DESTROY(Distributed_editing_dialog)(&distributed_editing_dialog);
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"CREATE(Distributed_editing_dialog).  "
          "Unable to allocate memory for the object");
        distributed_editing_dialog =
          (struct Distributed_editing_dialog *)NULL;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"CREATE(Distributed_editing_dialog).  "
        "Unable to open the dialog hierarchy");
      distributed_editing_dialog =
        (struct Distributed_editing_dialog *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Distributed_editing_dialog).  "
      "Invalid argument(s)");
    distributed_editing_dialog =
      (struct Distributed_editing_dialog *)NULL;
  }
  LEAVE;
  return(distributed_editing_dialog);
} /* CREATE(Distributed_editing_dialog)() */

int DESTROY(Distributed_editing_dialog)(
  struct Distributed_editing_dialog **dialog_address)
/*******************************************************************************
LAST MODIFIED : 13 January 2001

DESCRIPTION :
Destroys a distributed editing dialog object.
==============================================================================*/
{
  int return_code = 0;
  struct Distributed_editing_dialog *dialog;

  ENTER(DESTROY(Distributed_editing_dialog));
  if (dialog_address && (dialog = *dialog_address))
  {
    /* Make sure the activation toggle button gets turned off */
    if (dialog->activation)
    {
      XtVaSetValues(dialog->activation,
        XmNset,False,
        NULL);
    }
    /* make sure the window isn't currently being displayed */
    if (dialog->shell)
    {
      XtPopdown(dialog->shell);
      /* remove the dialog from the shell list */
      destroy_Shell_list_item_from_shell(&(dialog->shell),
        dialog->user_interface);
      /* destroying the shell should destroy all the children */
      XtDestroyWidget(dialog->window);
    }
    DEALLOCATE(*dialog_address);
    *dialog_address = (struct Distributed_editing_dialog *)NULL;
    return_code = 1;
  }
  LEAVE;
  return(return_code);
} /* DESTROY(Distributed_editing_dialog)() */

int Distributed_editing_dialog_pop_up(
  struct Distributed_editing_dialog *dialog)
/*******************************************************************************
LAST MODIFIED : 13 January 2001

DESCRIPTION :
Brings up the <dialog>
==============================================================================*/
{
  int return_code = 0;
  
  ENTER(Distributed_editing_dialog_pop_up);
  if (dialog && dialog->shell)
  {
    XtPopup(dialog->shell,XtGrabNone);
    XtVaSetValues(dialog->shell,
      XmNiconic,False,
      NULL);
    return_code = 1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Distributed_editing_dialog_pop_up. "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Distributed_editing_dialog_pop_up() */

int Distributed_editing_dialog_pop_down(
  struct Distributed_editing_dialog *dialog)
/*******************************************************************************
LAST MODIFIED : 22 January 2001

DESCRIPTION :
Pops down the <dialog>
==============================================================================*/
{
  int return_code = 0;
  
  ENTER(Distributed_editing_dialog_pop_down);
  if (dialog && dialog->shell)
  {
    close_distributed_editing_dialog((Widget)NULL,(XtPointer)dialog,
      (XtPointer)NULL);
    return_code = 1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Distributed_editing_dialog_pop_down. "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Distributed_editing_dialog_pop_down() */

int Distributed_editing_dialog_get_activation_state(
  struct Distributed_editing_dialog *dialog)
/*******************************************************************************
LAST MODIFIED : 22 January 2001

DESCRIPTION :
Returns 1 if the distributed editing toggle is set, 0 otherwise. The
distributed editing toggle is stored as the activation widget for the dialog.
==============================================================================*/
{
  int return_code = 0;
  Boolean set;
  
  ENTER(Distributed_editing_dialog_get_activation_state);
  if (dialog && dialog->activation)
  {
    XtVaGetValues(dialog->activation,
      XmNset,&set,
      NULL);
    if (set)
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
    display_message(ERROR_MESSAGE,
      "Distributed_editing_dialog_get_activation_state. "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Distributed_editing_dialog_get_activation_state() */

#endif /* defined (CELL_DISTRIBUTED) */
