/*******************************************************************************
FILE : cell_export_dialog.c

LAST MODIFIED : 21 November 2001

DESCRIPTION :
The export dialog used to export cell variables to ipcell and ipmatc files
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

#include <string.h>
#if defined (MOTIF)
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Xm/MwmUtil.h>
#include <Xm/DialogS.h>
#include <Xm/Protocols.h>
#endif /* defined (MOTIF) */
#include "cell/cell_export_dialog.h"
#include "cell/cell_export_dialog.uidh"
#include "choose/choose_element_group.h"
#include "choose/choose_computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "finite_element/import_finite_element.h"
#include "user_interface/filedir.h"
#include "user_interface/gui_dialog_macros.h"

/*
Module variables
================
*/
#if defined (MOTIF)
static int export_dialog_hierarchy_open=0;
static MrmHierarchy export_dialog_hierarchy;
#endif /* defined (MOTIF) */

/*
Module types
============
*/
struct Cell_export_dialog
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
Used to store the information and widgets used to export ipcell and ipmatc
files from Cell.
==============================================================================*/
{
  Widget shell;
  Widget window;
  Widget group_chooser_form;
  Widget group_chooser_widget;
  Widget grid_field_chooser_form;
  Widget grid_field_chooser_widget;
  Widget ipcell_file_label;
  Widget ipmatc_file_label;
  char *ipcell_file_name;
  char *ipmatc_file_name;
  struct Cell_interface *interface;
  struct Cell_cmgui_interface *cmgui_interface;
  struct Distributed_editing_interface *distributed_editing_interface;
  struct User_interface *user_interface;
}; /* Cell_export_dialog */

/*
Module functions
================
*/
DECLARE_DIALOG_IDENTIFY_FUNCTION(cell_export_dialog, \
  Cell_export_dialog,group_chooser_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(cell_export_dialog, \
  Cell_export_dialog,grid_field_chooser_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(cell_export_dialog, \
  Cell_export_dialog,ipcell_file_label)
DECLARE_DIALOG_IDENTIFY_FUNCTION(cell_export_dialog, \
  Cell_export_dialog,ipmatc_file_label)

static int set_ipcell_cb(char *filename,XtPointer dialog_void)
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
Callback for the file selection dialog for the ipcell file in the export dialog.
==============================================================================*/
{
  int return_code = 0;
  struct Cell_export_dialog *dialog;

  ENTER(set_ipcell_cb);
  if (dialog = (struct Cell_export_dialog *)dialog_void)
  {
    /* set the label widget */
    if (dialog->ipcell_file_label)
    {
      XtVaSetValues(dialog->ipcell_file_label,
        XmNlabelString,XmStringCreateSimple(filename),
				NULL);
    }
    /* and save the file name */
    if (dialog->ipcell_file_name)
    {
      DEALLOCATE(dialog->ipcell_file_name);
    }
    if (ALLOCATE(dialog->ipcell_file_name,char,strlen(filename)+1))
    {
			strcpy(dialog->ipcell_file_name,filename);
      return_code = 1;
    }
    else
    {
      display_message(ERROR_MESSAGE,"set_ipcell_cb.  "
        "Unable to allocate memory for the file name");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"set_ipcell_cb.  "
      "Missing export dialog");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* set_ipcell_cb() */

static int set_ipmatc_cb(char *filename,XtPointer dialog_void)
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
Callback for the file selection dialog for the ipmatc file in the export dialog.
==============================================================================*/
{
  int return_code = 0;
  struct Cell_export_dialog *dialog;

  ENTER(set_ipmatc_cb);
  if (dialog = (struct Cell_export_dialog *)dialog_void)
  {
    /* set the label widget */
    if (dialog->ipmatc_file_label)
    {
      XtVaSetValues(dialog->ipmatc_file_label,
        XmNlabelString,XmStringCreateSimple(filename),
				NULL);
    }
    /* and save the file name */
    if (dialog->ipmatc_file_name)
    {
      DEALLOCATE(dialog->ipmatc_file_name);
    }
    if (ALLOCATE(dialog->ipmatc_file_name,char,strlen(filename)+1))
    {
			strcpy(dialog->ipmatc_file_name,filename);
      return_code = 1;
    }
    else
    {
      display_message(ERROR_MESSAGE,"set_ipmatc_cb.  "
        "Unable to allocate memory for the file name");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"set_ipmatc_cb.  "
      "Missing export dialog");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* set_ipmatc_cb() */

static void ipcell_browse_cb(Widget widget,XtPointer dialog_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
Callback for the browse button for the ipcell file.
==============================================================================*/
{
  struct Cell_export_dialog *dialog;
  struct File_open_data *file_open_data;
  
  ENTER(ipcell_browse_cb);
  if (dialog = (struct Cell_export_dialog *)dialog_void)
  {
    if (file_open_data = create_File_open_data(".ipcell",REGULAR,
      set_ipcell_cb,(XtPointer)dialog,0,dialog->user_interface))
    {
      open_file_and_write(widget,(XtPointer)file_open_data,call_data);
      destroy_File_open_data(&file_open_data);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"ipcell_browse_cb. "
      "Missing dialog window");
  }
  LEAVE;
} /* ipcell_browse_cb() */

static void ipmatc_browse_cb(Widget widget,XtPointer dialog_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
Callback for the browse button for the ipmatc file.
==============================================================================*/
{
  struct Cell_export_dialog *dialog;
  struct File_open_data *file_open_data;
  
  ENTER(ipmatc_browse_cb);
  if (dialog = (struct Cell_export_dialog *)dialog_void)
  {
    if (file_open_data = create_File_open_data(".ipmatc",REGULAR,
      set_ipmatc_cb,(XtPointer)dialog,0,dialog->user_interface))
    {
      open_file_and_write(widget,(XtPointer)file_open_data,call_data);
      destroy_File_open_data(&file_open_data);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"ipmatc_browse_cb. "
      "Missing dialog window");
  }
  LEAVE;
} /* ipmatc_browse_cb() */

static void ok_button_cb(Widget widget,XtPointer dialog_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
Callback for the OK button in the export dialog.
==============================================================================*/
{
  struct Cell_export_dialog *dialog;
  struct GROUP(FE_element) *element_group;
  struct Computed_field *grid_field;
  struct FE_field *grid_fe_field;
  
  ENTER(ok_button_cb);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (dialog = (struct Cell_export_dialog *)dialog_void)
  {
    if (dialog->shell)
    {
      XtPopdown(dialog->shell);
      /* get the current element group and the grid field*/
      if ((element_group = CHOOSE_OBJECT_GET_OBJECT(GROUP(FE_element))(
				dialog->group_chooser_widget)) &&
        (grid_field = CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
					dialog->grid_field_chooser_widget)) &&
        Computed_field_get_type_finite_element(grid_field,&grid_fe_field))
      {
        if (Cell_interface_export_to_ipcell(dialog->interface,
          dialog->ipcell_file_name))
        {
          if (Cell_interface_export_to_ipmatc(dialog->interface,
            dialog->ipmatc_file_name,(void *)element_group,
            (void *)grid_fe_field))
          {
            /*display_message(INFORMATION_MESSAGE,"Done.\n");*/
          }
          else
          {
            display_message(ERROR_MESSAGE,"ok_button_cb. "
              "Unable to export an ipmatc file");
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"ok_button_cb. "
            "Unable to export an ipcell file");
        }
      }
      else
      {
				display_message(ERROR_MESSAGE,"ok_button_cb. "
          "Unable to get the element group");
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"ok_button_cb. "
        "Missing dialog shell");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"ok_button_cb. "
      "Missing export dialog");
  }
  LEAVE;
} /* ok_button_cb() */

static void cancel_button_cb(Widget widget,XtPointer dialog_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
Callback for the CANCEL button in the export dialog
==============================================================================*/
{
  struct Cell_export_dialog *dialog;
  
  ENTER(cancel_button_cb);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (dialog = (struct Cell_export_dialog *)dialog_void)
  {
    if (dialog->shell)
    {
      XtPopdown(dialog->shell);
    }
    else
    {
      display_message(ERROR_MESSAGE,"cancel_button_cb. "
        "Missing dialog shell");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"cancel_button_cb. "
      "Missing export dialog");
  }
  LEAVE;
} /* cancel_button_cb() */

static void export_dialog_destroy_cb(Widget widget,XtPointer dialog_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
Callback for when the export dialog is destroyed via the window manager menu ??
==============================================================================*/
{
  struct Cell_export_dialog *dialog;
  
  ENTER(export_dialog_destroy_cb);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (dialog = (struct Cell_export_dialog *)dialog_void)
  {
    if (dialog->shell)
    {
      /* make sure the dialog is no longer visible */
      XtPopdown(dialog->shell);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"export_dialog_destroy_cb. "
      "Missing export dialog");
  }
  LEAVE;
} /* export_dialog_destroy_cb() */

/*
Global functions
================
*/
struct Cell_export_dialog *CREATE(Cell_export_dialog)(
  struct Cell_interface *interface,
  struct Cell_cmgui_interface *cmgui_interface,
  struct Distributed_editing_interface *distributed_editing_interface,
  struct User_interface *user_interface,Widget parent)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
Create a new export dialog.
==============================================================================*/
{
  struct Cell_export_dialog *dialog;
  Atom WM_DELETE_WINDOW;
  MrmType export_dialog_class;
  static MrmRegisterArg callback_list[] = {
    {"id_group_chooser_form",
     (XtPointer)DIALOG_IDENTIFY(cell_export_dialog,group_chooser_form)},
    {"id_grid_field_chooser_form",
     (XtPointer)DIALOG_IDENTIFY(cell_export_dialog,grid_field_chooser_form)},
    {"id_ipcell_file_label",
     (XtPointer)DIALOG_IDENTIFY(cell_export_dialog,ipcell_file_label)},
    {"id_ipmatc_file_label",
     (XtPointer)DIALOG_IDENTIFY(cell_export_dialog,ipmatc_file_label)},
    {"ipcell_browse_CB",(XtPointer)ipcell_browse_cb},
    {"ipmatc_browse_CB",(XtPointer)ipmatc_browse_cb},
    {"ok_button_CB",(XtPointer)ok_button_cb},
    {"cancel_button_CB",(XtPointer)cancel_button_cb}
  }; /* callback_list */
  static MrmRegisterArg identifier_list[] = {
    {"dialog_structure",(XtPointer)NULL},
    {"default_ipcell_file_name",(XtPointer)NULL},
    {"default_ipmatc_file_name",(XtPointer)NULL}
  }; /* identifier_list */
  struct MANAGER(Computed_field) *computed_field_manager;
	struct Computed_field *grid_field;
  
  ENTER(CREATE(Cell_export_dialog));
  if (cmgui_interface && distributed_editing_interface && interface &&
    user_interface && parent)
  {
    if (MrmOpenHierarchy_base64_string(cell_export_dialog_uidh,
      &export_dialog_hierarchy,&export_dialog_hierarchy_open))
    {
      if (ALLOCATE(dialog,struct Cell_export_dialog,1))
      {
        /* initialise the structure */
        dialog->shell = (Widget)NULL;
        dialog->window = (Widget)NULL;
        dialog->group_chooser_form = (Widget)NULL;
        dialog->group_chooser_widget = (Widget)NULL;
        dialog->ipcell_file_label = (Widget)NULL;
        dialog->ipcell_file_name = (char *)NULL;
        dialog->ipmatc_file_label = (Widget)NULL;
        dialog->ipmatc_file_name = (char *)NULL;
        dialog->interface = interface;
        dialog->cmgui_interface = cmgui_interface;
        dialog->distributed_editing_interface = distributed_editing_interface;
        dialog->user_interface = user_interface;
        /* set the default file names */
        if (ALLOCATE(dialog->ipcell_file_name,char,strlen("cell.ipcell")+1) && 
          ALLOCATE(dialog->ipmatc_file_name,char,strlen("cell.ipmatc")+1))
        {
          strcpy(dialog->ipcell_file_name,"cell.ipcell");
          strcpy(dialog->ipmatc_file_name,"cell.ipmatc");
        }
        /* make the dialog shell */
        if (dialog->shell = XtVaCreatePopupShell("export_dialog_shell",
          xmDialogShellWidgetClass,parent,
          XmNmwmDecorations,MWM_DECOR_ALL,
          XmNmwmFunctions,MWM_FUNC_ALL,
          XmNdeleteResponse,XmDESTROY,
          XmNtransient,FALSE,
          NULL))
        {
          /* identify the shell for the busy icon */
          create_Shell_list_item(&(dialog->shell),user_interface);
          /* add the destroy callback */
          XtAddCallback(dialog->shell,XmNdestroyCallback,
            export_dialog_destroy_cb,(XtPointer)dialog);
          WM_DELETE_WINDOW = XmInternAtom(XtDisplay(dialog->shell),
            "WM_DELETE_WINDOW",FALSE);
          XmAddWMProtocolCallback(dialog->shell,WM_DELETE_WINDOW,
            export_dialog_destroy_cb,(XtPointer)dialog);
          /* register cellbacks in UIL */
          if (MrmSUCCESS == MrmRegisterNamesInHierarchy(
            export_dialog_hierarchy,callback_list,XtNumber(callback_list)))
          {
            /* set the identifier's values */
            identifier_list[0].value = (XtPointer)dialog;
            identifier_list[1].value = (XtPointer)
              (XmStringCreateSimple(dialog->ipcell_file_name));
            identifier_list[2].value = (XtPointer)
              (XmStringCreateSimple(dialog->ipmatc_file_name));
            /* register the identifiers */
            if (MrmSUCCESS == MrmRegisterNamesInHierarchy(
              export_dialog_hierarchy,identifier_list,
              XtNumber(identifier_list)))
            {
              /* fetch the window widget */
              if (MrmSUCCESS == MrmFetchWidget(export_dialog_hierarchy,
                "export_dialog",dialog->shell,
                &(dialog->window),&export_dialog_class))
              {
                /* create the element group chooser */
                if (dialog->group_chooser_widget =
                  CREATE_CHOOSE_OBJECT_WIDGET(GROUP(FE_element))(
                    dialog->group_chooser_form,
                    (struct GROUP(FE_element) *)NULL,
                    Cell_cmgui_interface_get_element_group_manager(
                      cmgui_interface),
                    (MANAGER_CONDITIONAL_FUNCTION(GROUP(FE_element)) *)NULL,
                    (void *)NULL, user_interface))
                {
                  /* create the grid field chooser - used to set the field
                   * which will be used as the grid point number field
                   * when exporting the ipmatc file.
                   * First set the initial grid field to use for selecting
                   * element points - if a grid_point_number field exists
                   * use that, otherwise just take the first on which is
                   * suitable
                   */
                  if (Distributed_editing_interface_has_element_copy(
                    distributed_editing_interface))
                  {
                    computed_field_manager =
                      Cell_cmgui_interface_get_computed_field_manager(
                        cmgui_interface);
                    if (!(grid_field =
                      FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
                        "grid_point_number",computed_field_manager)))
                    {
                      grid_field = FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
                        Computed_field_is_scalar_integer_grid_in_element,
                        (void *)Distributed_editing_interface_get_element_copy(
                          distributed_editing_interface),
                        computed_field_manager);
                    }
                    if (dialog->grid_field_chooser_widget =
                      CREATE_CHOOSE_OBJECT_WIDGET(Computed_field)(
                        dialog->grid_field_chooser_form,grid_field,
                        computed_field_manager,
                        Computed_field_is_scalar_integer_grid_in_element,
                        (void *)Distributed_editing_interface_get_element_copy(
                          distributed_editing_interface), user_interface))
                    {
                      XtManageChild(dialog->window);
                      XtRealizeWidget(dialog->shell);
                    }
                    else
                    {
                      display_message(ERROR_MESSAGE,
                        "CREATE(Cell_export_dialog).  "
                        "Unable to create the grid field chooser widget");
                      DESTROY(Cell_export_dialog)(&dialog);
                      dialog = (struct Cell_export_dialog *)NULL;
                    }
                  }
                  else
                  {
                    display_message(ERROR_MESSAGE,
                      "CREATE(Cell_export_dialog).  "
                      "No elements defined ????");
                    DESTROY(Cell_export_dialog)(&dialog);
                    dialog = (struct Cell_export_dialog *)NULL;
                  }
                }
                else
                {
                  display_message(ERROR_MESSAGE,"CREATE(Cell_export_dialog).  "
                    "Unable to create the group chooser widget");
                  DESTROY(Cell_export_dialog)(&dialog);
                  dialog = (struct Cell_export_dialog *)NULL;
                }
              }
              else
              {
                display_message(ERROR_MESSAGE,"CREATE(Cell_export_dialog).  "
                  "Unable to fetch the window widget");
                DESTROY(Cell_export_dialog)(&dialog);
                dialog = (struct Cell_export_dialog *)NULL;
              }
            }
            else
            {
              display_message(ERROR_MESSAGE,"CREATE(Cell_export_dialog).  "
                "Unable to register the identifiers");
              DESTROY(Cell_export_dialog)(&dialog);
              dialog = (struct Cell_export_dialog *)NULL;
            }
          }
          else
          {
            display_message(ERROR_MESSAGE,"CREATE(Cell_export_dialog).  "
              "Unable to register the callbacks");
            DESTROY(Cell_export_dialog)(&dialog);
            dialog = (struct Cell_export_dialog *)NULL;
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"CREATE(Cell_export_dialog).  "
            "Unable to create the dialog shell for the export dialog");
          DESTROY(Cell_export_dialog)(&dialog);
          dialog = (struct Cell_export_dialog *)NULL;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"CREATE(Cell_export_dialog).  "
          "Unable to allocate memory for the export dialog");
        dialog = (struct Cell_export_dialog *)NULL;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"CREATE(Cell_export_dialog).  "
        "Unable to open the export dialog hierarchy");
      dialog = (struct Cell_export_dialog *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Cell_export_dialog).  "
      "Invalid argument(s)");
    dialog = (struct Cell_export_dialog *)NULL;
  }
  LEAVE;
  return(dialog);
} /* CREATE(Cell_export_dialog)() */

int DESTROY(Cell_export_dialog)(struct Cell_export_dialog **dialog_address)
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
Destroys a export dialog object.
==============================================================================*/
{
  int return_code = 0;
  struct Cell_export_dialog *dialog;

  ENTER(DESTROY(Cell_export_dialog));
  if (dialog_address && (dialog = *dialog_address))
  {
    /* Clear out any memory */
    if (dialog->ipcell_file_name != (char *)NULL)
    {
      DEALLOCATE(dialog->ipcell_file_name);
    }
    if (dialog->ipmatc_file_name != (char *)NULL)
    {
      DEALLOCATE(dialog->ipmatc_file_name);
    }
    /* make sure the window isn't currently being displayed */
    if (dialog->shell)
    {
      XtPopdown(dialog->shell);
      /* remove the dialog from the shell list */
      destroy_Shell_list_item_from_shell(&(dialog->shell),
        dialog->user_interface);
      /* destroying the shell should destroy all the children */
      XtDestroyWidget(dialog->shell);
    }
    DEALLOCATE(*dialog_address);
    *dialog_address = (struct Cell_export_dialog *)NULL;
    return_code = 1;
  }
  LEAVE;
  return(return_code);
} /* DESTROY(Cell_export_dialog)() */

int Cell_export_dialog_pop_up(struct Cell_export_dialog *dialog)
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
Pops up the <dialog>
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_export_dialog_pop_up);
  if (dialog)
  {
    if (dialog->shell)
    {
      XtPopup(dialog->shell,XtGrabNone);
      return_code = 1;
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_export_dialog_pop_up.  "
        "Missing dialog shell");
      return_code = 0;
    } 
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_export_dialog_pop_up.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_export_dialog_pop_up() */

int Cell_export_dialog_pop_down(struct Cell_export_dialog *dialog)
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
Pops down the <dialog>
==============================================================================*/
{
  int return_code = 0;

  ENTER(Cell_export_dialog_pop_down);
  if (dialog)
  {
    if (dialog->shell)
    {
      XtPopdown(dialog->shell);
      return_code = 1;
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_export_dialog_pop_down.  "
        "Missing dialog shell");
      return_code = 0;
    } 
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_export_dialog_pop_down.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_export_dialog_pop_down() */

#endif /* defined (CELL_DISTRIBUTED) */
