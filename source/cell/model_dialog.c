/*******************************************************************************
FILE : model_dialog.c

LAST MODIFIED : 16 September 1999

DESCRIPTION :
Functions and structures for using the model dialog box.
==============================================================================*/
#include <stdio.h>
#if defined (MOTIF)
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Xm/MwmUtil.h>
#include <Xm/DialogS.h>
#include <Xm/List.h>
#include <Xm/Protocols.h>
#endif /* if defined (MOTIF) */
#include "user_interface/user_interface.h"
#include "user_interface/filedir.h"
#include "cell/model_dialog.h"
#include "cell/model_dialog.uidh"
#include "cell/cell_window.h"

/*
Module types
============
*/
#if defined (MOTIF)
typedef struct
/*******************************************************************************
LAST MODIFIED : 14 February 1999

DESCRIPTION :
Holds the information related to the models dialog specified in the resource
file.
==============================================================================*/
{
	char *models_file;
} User_settings;
#endif /* defined (MOTIF) */

/*
Module variables
================
*/
#if defined (MOTIF)
static int model_dialog_hierarchy_open=0;
static MrmHierarchy model_dialog_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
================
*/
static void identify_current_model_label(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 February 1999

DESCRIPTION :
Grabs the id of the currnet model label widget.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;

  ENTER(identify_current-model_label);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    cell->model_dialog->current_model_label = widget;
  }
  else
  {
    display_message(ERROR_MESSAGE,"identify_current_model_label. "
      "Missing Cell window");
  }
  LEAVE;
} /* END identify_current_model_label() */

static void identify_membrane_list(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 04 February 1999

DESCRIPTION :
Grabs the id of the membrane list widget.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;

  ENTER(identify_membrane_list);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    cell->model_dialog->membrane_list = widget;
  }
  else
  {
    display_message(ERROR_MESSAGE,"identify_membrane_list. "
      "Missing Cell window");
  }
  LEAVE;
} /* END identify_membrane_list() */

static void identify_mechanics_list(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 04 February 1999

DESCRIPTION :
Grabs the id of the mechanics list widget.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;

  ENTER(identify_mechanics_list);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    cell->model_dialog->mechanics_list = widget;
  }
  else
  {
    display_message(ERROR_MESSAGE,"identify_mechanics_list. "
      "Missing Cell window");
  }
  LEAVE;
} /* END identify_mechanics_list() */

static void identify_metabolism_list(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 04 February 1999

DESCRIPTION :
Grabs the id of the metabolism list widget.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;

  ENTER(identify_metabolism_list);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    cell->model_dialog->metabolism_list = widget;
  }
  else
  {
    display_message(ERROR_MESSAGE,"identify_metabolism_list. "
      "Missing Cell window");
  }
  LEAVE;
} /* END identify_metabolism_list() */

static void identify_coupled_list(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 04 February 1999

DESCRIPTION :
Grabs the id of the coupled list widget.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;

  ENTER(identify_coupled_list);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    cell->model_dialog->coupled_list = widget;
  }
  else
  {
    display_message(ERROR_MESSAGE,"identify_coupled_list. "
      "Missing Cell window");
  }
  LEAVE;
} /* END identify_coupled_list() */

static void update_current_model(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 28 February 1999

DESCRIPTION :
Updates the cell->current_model from the
cell->model_dialog->current_model_string and resets the
cell->model_dialog->current_model_label widget.
==============================================================================*/
{
  char *str;
  char start[] = {"Current model = "};
  
  ENTER(update_current_model);
  if (cell->model_dialog->current_model_label)
  {
    if (cell->model_dialog->current_model_string == (char *)NULL)
    {
      if (ALLOCATE(str,char,strlen(start)+strlen("??")+1))
      {
        sprintf(str,"%s??\0",start);
        XtVaSetValues(cell->model_dialog->current_model_label,
          XmNlabelString,XmStringCreateSimple(str),
          NULL);
        DEALLOCATE(str);
      }
      else
      {
        display_message(ERROR_MESSAGE,"update_current_model. "
          "Unable to allocate memory for the label string");
      }
    }
    else
    {
      if ((ALLOCATE(str,char,strlen(start)+
        strlen(cell->model_dialog->current_model_string)+1)) &&
        (ALLOCATE(cell->current_model,char,
          strlen(cell->model_dialog->current_model_string)+1)))
      {
        sprintf(cell->current_model,"%s\0",
          cell->model_dialog->current_model_string);
        sprintf(str,"%s%s\0",start,cell->model_dialog->current_model_string);
        XtVaSetValues(cell->model_dialog->current_model_label,
          XmNlabelString,XmStringCreateSimple(str),
          NULL);
        DEALLOCATE(str);
      }
      else
      {
        display_message(ERROR_MESSAGE,"update_current_model. "
          "Unable to allocate memory for the label string");
      }
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"update_current_model. "
      "Missing label widget");
  }
  LEAVE;
} /* END update_current_model() */

static int find_model_name_and_read(char *model_name,struct Cell_window *cell,
  struct Model_name *names)
/*******************************************************************************
LAST MODIFIED : 14 September 1999

DESCRIPTION :
Searches the <names> array for the <model_name>, and then reads the model in
from the model's uri. Also sets cell->current_model_id from the model name.
==============================================================================*/
{
  int return_code = 0;
  struct Model_name *current = (struct Model_name *)NULL;
  char *filename;

  ENTER(find_model_name_and_read);
  if (model_name && cell && names)
  {
    /* find the appropriate model name struct. */
    current = names;
    while ((current != (struct Model_name *)NULL) &&
      strncmp(current->name,model_name,strlen(model_name)))
    {
      current = current->next;
    }
    if (current != (struct Model_name *)NULL)
    {
      /* now set the filename and read in */
      if (ALLOCATE(filename,char,
        strlen(current->uri->path)+strlen(current->uri->filename)+1))
      {
        sprintf(filename,"%s%s\0",current->uri->path,current->uri->filename);
        cell->default_values = 1;
        return_code = read_model_file(filename,(XtPointer)cell);
        if (return_code)
        {
          /*if (cell->current_model_id)
          {
            DEALLOCATE(cell->current_model_id);
          }*/
          if (ALLOCATE(cell->current_model_id,char,strlen(current->id)+1))
          {
            sprintf(cell->current_model_id,"%s\0",current->id);
            /* update from node, if required */
            if ((cell->distributed).edit)
            {
              update_cell_window_from_node(cell);
            }
          }
          else
          {
            display_message(ERROR_MESSAGE,"find_model_name_and_read. "
              "Unable to allocate memory for the current model ID");
            return_code = 0;
          }
        }
        cell->default_values = 0;
      }
      else
      {
        display_message(ERROR_MESSAGE,"find_model_name_and_read. "
          "Unable to allocate memory for the filename");
        return_code = 0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"find_model_name_and_read. "
        "Unable to find corresponding model name");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"find_model_name_and_read. "
      "Invalid arguments");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END find_model_name_and_read() */

static void default_button_callback(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 February 1999

DESCRIPTION :
Callback for the "Read default.." button in the model dialog
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;

  ENTER(default_button_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (cell->model_dialog && cell->model_dialog->shell)
    {
      busy_cursor_off(cell->model_dialog->shell,cell->user_interface);
      /*display_message(INFORMATION_MESSAGE,"default_button_callback. "
        "busy cursor off\n");*/
      XtPopdown(cell->model_dialog->shell);
      if (cell->model_dialog->current_model_string)
      {
        /* find the appropriate model name and read the corresponding
           model file */
        if (find_model_name_and_read(cell->model_dialog->current_model_string,
          cell,cell->model_dialog->model_names))
        {
          /*display_message(INFORMATION_MESSAGE,"default_button_callback. "
            "Current model is : **%s**\n",
            cell->model_dialog->current_model_string);*/
          /* update the current model string and the current model label */
          update_current_model(cell);
        }
        else
        {
          display_message(ERROR_MESSAGE,"default_button_callback. "
            "Unable to find or read the appropriate model file");
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"default_button_callback. "
          "Missing current model string");
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"default_button_callback. "
        "Missing model dialog");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"default_button_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END default_button_callback() */

static void read_button_callback(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 February 1999

DESCRIPTION :
Callback for the "Read model from file" button in the model dialog. First set
the defaults for the model, then prompt the user for a model file.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  struct File_open_data *model_file_open_data;
  
  ENTER(read_button_callback);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (cell->model_dialog && cell->model_dialog->shell)
    {
      busy_cursor_off(cell->model_dialog->shell,cell->user_interface);
      /*display_message(INFORMATION_MESSAGE,"read_button_callback. "
        "busy cursor off\n");*/
      XtPopdown(cell->model_dialog->shell);
      if (cell->model_dialog->current_model_string)
      {
        /* find the appropriate model name and read the corresponding
           model file */
        if (find_model_name_and_read(cell->model_dialog->current_model_string,
          cell,cell->model_dialog->model_names))
        {
          /*display_message(INFORMATION_MESSAGE,"read_button_callback. "
            "Current model is : **%s**\n",
            cell->model_dialog->current_model_string);*/
          /* update the current model string and the current model label */
          update_current_model(cell);
          /* now prompt the user for their own file */
          cell->default_values = 0;
          if (model_file_open_data = create_File_open_data(".model",
            REGULAR,read_model_file,(XtPointer)cell,0,cell->user_interface))
          {
            open_file_and_read(widget,(XtPointer)model_file_open_data,
              (XtPointer)NULL);
          }
          else
          {
            display_message(ERROR_MESSAGE,"read_button_callback. "
              "Unable to create the file open data structure");
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"read_button_callback. "
            "Unable to find or read the appropriate model file");
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"read_button_callback. "
          "Missing current model string");
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"read_button_callback. "
        "Missing model dialog");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"read_button_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END read_button_callback() */

static void cancel_button_callback(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 04 February 1999

DESCRIPTION :
Callback for the "Cancel" button in the model dialog
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;

  ENTER(cancel_button_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (cell->model_dialog && cell->model_dialog->shell)
    {
      busy_cursor_off(cell->model_dialog->shell,cell->user_interface);
      /*display_message(INFORMATION_MESSAGE,"cancel_button_callback. "
        "busy cursor off\n");*/
      /*display_message(INFORMATION_MESSAGE,"cancel_button_callback. "
        "Current model is : **%s**\n",
        cell->model_dialog->current_model_string);*/
      XtPopdown(cell->model_dialog->shell);
    }
    else
    {
      display_message(ERROR_MESSAGE,"cancel_button_callback. "
        "Missing model dialog");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"cancel_button_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END cancel_button_callback() */

static void help_button_callback(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 04 February 1999

DESCRIPTION :
Callback for the "Help" button in the model dialog
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;

  ENTER(help_button_callback);
  USE_PARAMETER(cell);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    display_message(INFORMATION_MESSAGE,"help_button_callback. "
      "Sorry, no help for the models dialog\n");
  }
  else
  {
    display_message(ERROR_MESSAGE,"help_button_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END help_button_callback() */

static void model_dialog_destroy_callback(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 03 February 1999

DESCRIPTION :
Callback for when the model dialog is destroy via the window manager menu ??
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(model_dialog_destroy_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (cell->model_dialog)
    {
      if (cell->model_dialog->shell)
      {
        busy_cursor_off(cell->model_dialog->shell,cell->user_interface);
        /*display_message(INFORMATION_MESSAGE,"model_dialog_destroy_callback. "
          "busy cursor off\n");*/
        /* remove the shell from the shell list */
        destroy_Shell_list_item_from_shell(&(cell->model_dialog->shell),
          cell->user_interface);
        /* make sure the dialog is no longer visible */
        XtPopdown(cell->model_dialog->shell);
        /* Unmanage the shell */
        XtUnmanageChild(cell->model_dialog->shell);
        /*display_message(INFORMATION_MESSAGE,"model_dialog_destroy_callback. "
          "Unmanage model_dialog->shell\n");*/
      }
      DEALLOCATE(cell->model_dialog);
      cell->model_dialog = (struct Model_dialog *)NULL;
      /*display_message(INFORMATION_MESSAGE,"model_dialog_destroy_callback. "
        "DEALLOCATE model_dialog\n");*/
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"model_dialog_destroy_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END model_dialog_destroy_callback() */

static void default_action_callback(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 February 1999

DESCRIPTION :
Callback for any "default action" (double click) in any of the model dialog
list widgets.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  XmListCallbackStruct *cbs = (XmListCallbackStruct *)NULL;

  ENTER(default_action_callback);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (cbs = ((XmListCallbackStruct *)call_data))
    {
      /* check for the model list widgets */
      if (cell->model_dialog && cell->model_dialog->shell &&
        cell->model_dialog->membrane_list &&
        cell->model_dialog->mechanics_list &&
        cell->model_dialog->metabolism_list &&
        cell->model_dialog->coupled_list)
      {
        /* pop down the model dialog */
        XtPopdown(cell->model_dialog->shell);
        busy_cursor_off(cell->model_dialog->shell,cell->user_interface);
        /* deselect all items in all the lists */
        XmListDeselectAllItems(cell->model_dialog->membrane_list);
        XmListDeselectAllItems(cell->model_dialog->mechanics_list);
        XmListDeselectAllItems(cell->model_dialog->metabolism_list);
        XmListDeselectAllItems(cell->model_dialog->coupled_list);
        /* select the specified item, passing False so that the selection
           callback is not called */
        XmListSelectItem(widget,cbs->item,False);
        XmStringGetLtoR(cbs->item,XmSTRING_DEFAULT_CHARSET,
          &(cell->model_dialog->current_model_string));
        /* find the appropriate model name and read the corresponding
           model file */
        if (find_model_name_and_read(cell->model_dialog->current_model_string,
          cell,cell->model_dialog->model_names))
        {
          /*display_message(INFORMATION_MESSAGE,"default_action_callback. "
            "Current model is : **%s**\n",
            cell->model_dialog->current_model_string);*/
          /* update the current model string and the current model label */
          update_current_model(cell);
        }
        else
        {
          display_message(ERROR_MESSAGE,"default_action_callback. "
            "Unable to find or read the appropriate model file");
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"default_action_callback. "
          "Missing a model list");
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"default_action_callback. "
        "Missing callback structure");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"default_action_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END default_action_callback() */

static void membrane_item_selected(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 04 February 1999

DESCRIPTION :
Callback for when an item is selected in the membrane model list.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  XmListCallbackStruct *cbs = (XmListCallbackStruct *)NULL;

  ENTER(membrane_item_selected);
  USE_PARAMETER(widget);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (cbs = ((XmListCallbackStruct *)call_data))
    {
      /* check for the model list widgets */
      if (cell->model_dialog && cell->model_dialog->shell &&
        cell->model_dialog->membrane_list &&
        cell->model_dialog->mechanics_list &&
        cell->model_dialog->metabolism_list &&
        cell->model_dialog->coupled_list)
      {
        /* deselect all items in all the other lists */
        XmListDeselectAllItems(cell->model_dialog->mechanics_list);
        XmListDeselectAllItems(cell->model_dialog->metabolism_list);
        XmListDeselectAllItems(cell->model_dialog->coupled_list);
        /* set the current model string */
        XmStringGetLtoR(cbs->item,XmSTRING_DEFAULT_CHARSET,
          &(cell->model_dialog->current_model_string));
      }
      else
      {
        display_message(ERROR_MESSAGE,"membrane_item_selected. "
          "Missing a model list");
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"membrane_item_selected. "
        "Missing callback structure");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"membrane_item_selected. "
      "Missing Cell window");
  }
  LEAVE;
} /* END membrane_item_selected() */

static void mechanics_item_selected(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 04 February 1999

DESCRIPTION :
Callback for when an item is selected in the mechanics model list.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  XmListCallbackStruct *cbs = (XmListCallbackStruct *)NULL;

  ENTER(mechanics_item_selected);
  USE_PARAMETER(widget);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (cbs = ((XmListCallbackStruct *)call_data))
    {
      /* check for the model list widgets */
      if (cell->model_dialog && cell->model_dialog->shell &&
        cell->model_dialog->membrane_list &&
        cell->model_dialog->mechanics_list &&
        cell->model_dialog->metabolism_list &&
        cell->model_dialog->coupled_list)
      {
        /* deselect all items in all the other lists */
        XmListDeselectAllItems(cell->model_dialog->membrane_list);
        XmListDeselectAllItems(cell->model_dialog->metabolism_list);
        XmListDeselectAllItems(cell->model_dialog->coupled_list);
        /* set the current model string */
        XmStringGetLtoR(cbs->item,XmSTRING_DEFAULT_CHARSET,
          &(cell->model_dialog->current_model_string));
      }
      else
      {
        display_message(ERROR_MESSAGE,"mechanics_item_selected. "
          "Missing a model list");
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"mechanics_item_selected. "
        "Missing callback structure");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"mechanics_item_selected. "
      "Missing Cell window");
  }
  LEAVE;
} /* END mechanics_item_selected() */

static void metabolism_item_selected(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 04 February 1999

DESCRIPTION :
Callback for when an item is selected in the metabolism model list.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  XmListCallbackStruct *cbs = (XmListCallbackStruct *)NULL;

  ENTER(metabolism_item_selected);
  USE_PARAMETER(widget);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (cbs = ((XmListCallbackStruct *)call_data))
    {
      /* check for the model list widgets */
      if (cell->model_dialog && cell->model_dialog->shell &&
        cell->model_dialog->membrane_list &&
        cell->model_dialog->mechanics_list &&
        cell->model_dialog->metabolism_list &&
        cell->model_dialog->coupled_list)
      {
        /* deselect all items in all the other lists */
        XmListDeselectAllItems(cell->model_dialog->membrane_list);
        XmListDeselectAllItems(cell->model_dialog->mechanics_list);
        XmListDeselectAllItems(cell->model_dialog->coupled_list);
        /* set the current model string */
        XmStringGetLtoR(cbs->item,XmSTRING_DEFAULT_CHARSET,
          &(cell->model_dialog->current_model_string));
      }
      else
      {
        display_message(ERROR_MESSAGE,"metabolism_item_selected. "
          "Missing a model list");
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"metabolism_item_selected. "
        "Missing callback structure");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"metabolism_item_selected. "
      "Missing Cell window");
  }
  LEAVE;
} /* END metabolism_item_selected() */

static void coupled_item_selected(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 04 February 1999

DESCRIPTION :
Callback for when an item is selected in the coupled model list.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  XmListCallbackStruct *cbs = (XmListCallbackStruct *)NULL;

  ENTER(coupled_item_selected);
  USE_PARAMETER(widget);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (cbs = ((XmListCallbackStruct *)call_data))
    {
      /* check for the model list widgets */
      if (cell->model_dialog && cell->model_dialog->shell &&
        cell->model_dialog->membrane_list &&
        cell->model_dialog->mechanics_list &&
        cell->model_dialog->metabolism_list &&
        cell->model_dialog->coupled_list)
      {
        /* deselect all items in all the other lists */
        XmListDeselectAllItems(cell->model_dialog->membrane_list);
        XmListDeselectAllItems(cell->model_dialog->metabolism_list);
        XmListDeselectAllItems(cell->model_dialog->mechanics_list);
        /* set the current model string */
        XmStringGetLtoR(cbs->item,XmSTRING_DEFAULT_CHARSET,
          &(cell->model_dialog->current_model_string));
      }
      else
      {
        display_message(ERROR_MESSAGE,"coupled_item_selected. "
          "Missing a model list");
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"coupled_item_selected. "
        "Missing callback structure");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"coupled_item_selected. "
      "Missing Cell window");
  }
  LEAVE;
} /* END coupled_item_selected() */

static int add_model_names_to_list(Widget list,struct Model_name *names,
  enum Model_type type)
/*******************************************************************************
LAST MODIFIED : 27 February 1999

DESCRIPTION :
Add the appropriate <names> the the <list>.
==============================================================================*/
{
  int return_code = 0;
  struct Model_name *current = (struct Model_name *)NULL;

  ENTER(add_model_names_to_list);
  /* deselect and delete any existing items in the list */
  XmListDeselectAllItems(list);
  XmListDeleteAllItems(list);
  /* add the list */
  current = names;
  while (current != (struct Model_name *)NULL)
  {
    if (current->type == type)
    {
      XmListAddItemUnselected(list,
        XmStringCreateSimple(current->name),
        0); /* set the position to 0 so new items are added to end of list */
    }
    current = current->next;
  }
  return_code = 1;
  LEAVE;
  return(return_code);
} /* END add_model_names_to_list() */

static int add_model_names(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 05 February 1999

DESCRIPTION :
Add the model names to the appropriate widgets.
==============================================================================*/
{
  int return_code = 0,item_count = 0;

  ENTER(add_model_names);
  if (cell != (struct Cell_window *)NULL)
  {
    if (cell->model_dialog->membrane_list &&
      add_model_names_to_list(cell->model_dialog->membrane_list,
        cell->model_dialog->model_names,MEMBRANE_MODEL))
    {
      if (cell->model_dialog->mechanics_list &&
        add_model_names_to_list(cell->model_dialog->mechanics_list,
          cell->model_dialog->model_names,MECHANICS_MODEL))
      {
        if (cell->model_dialog->metabolism_list &&
          add_model_names_to_list(cell->model_dialog->metabolism_list,
            cell->model_dialog->model_names,METABOLISM_MODEL))
        {
          if (cell->model_dialog->coupled_list &&
            add_model_names_to_list(cell->model_dialog->coupled_list,
              cell->model_dialog->model_names,COUPLED_MODEL))
          {
            /* set the first item to be the selected item */
            XtVaGetValues(cell->model_dialog->membrane_list,
              XmNitemCount,&item_count,
              NULL);
            if (item_count > 0)
            {
              XmListSelectPos(cell->model_dialog->membrane_list,1,False);
            }
            else
            {
              XtVaGetValues(cell->model_dialog->mechanics_list,
                XmNitemCount,&item_count,
                NULL);
              if (item_count > 0)
              {
                XmListSelectPos(cell->model_dialog->mechanics_list,1,False);
              }
              else
              {
                XtVaGetValues(cell->model_dialog->metabolism_list,
                  XmNitemCount,&item_count,
                  NULL);
                if (item_count > 0)
                {
                  XmListSelectPos(cell->model_dialog->metabolism_list,1,False);
                }
                else
                {
                  XtVaGetValues(cell->model_dialog->coupled_list,
                    XmNitemCount,&item_count,
                    NULL);
                  if (item_count > 0)
                  {
                    XmListSelectPos(cell->model_dialog->coupled_list,1,False);
                  }
                }
              }
            }
            return_code = 1;
          }
          else
          {
            display_message(ERROR_MESSAGE,"add_model_names. "
              "Unable to add coupled models");
            return_code = 0;
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"add_model_names. "
            "Unable to add metabolism models");
          return_code = 0;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"add_model_names. "
          "Unable to add mechanics models");
        return_code = 0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"add_model_names. "
        "Unable to add membrane models");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"add_model_names. "
      "Missing Cell window");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END add_model_names() */

static int create_model_dialog(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 26 August 1999

DESCRIPTION :
Create a new model dialog.
==============================================================================*/
{
  int return_code = 0;
  Atom WM_DELETE_WINDOW;
  MrmType model_dialog_class;
  static MrmRegisterArg callback_list[] = {
    {"default_button_callback",(XtPointer)default_button_callback},
    {"read_button_callback",(XtPointer)read_button_callback},
    {"cancel_button_callback",(XtPointer)cancel_button_callback},
    {"help_button_callback",(XtPointer)help_button_callback},
    {"identify_current_model_label",(XtPointer)identify_current_model_label},
    {"identify_membrane_list",(XtPointer)identify_membrane_list},
    {"identify_mechanics_list",(XtPointer)identify_mechanics_list},
    {"identify_metabolism_list",(XtPointer)identify_metabolism_list},
    {"identify_coupled_list",(XtPointer)identify_coupled_list},
    {"default_action_callback",(XtPointer)default_action_callback},
    {"membrane_item_selected",(XtPointer)membrane_item_selected},
    {"mechanics_item_selected",(XtPointer)mechanics_item_selected},
    {"metabolism_item_selected",(XtPointer)metabolism_item_selected},
    {"coupled_item_selected",(XtPointer)coupled_item_selected}
  }; /* callback_list */
  static MrmRegisterArg identifier_list[] = {
    {"cell_window_structure",(XtPointer)NULL},
    {"text_emphasis_colour",(XtPointer)NULL}
  }; /* identifier_list */
#if defined (MOTIF)
#define XmNmodelsFile "modelsFile"
#define XmCModelsFile "ModelsFile"
  static XtResource resources[] = {
    {
      XmNmodelsFile,
      XmCModelsFile,
      XmRString,
			sizeof(char *),
			XtOffsetOf(User_settings,models_file),
			XmRString,
			"/www/cellml/unreal/cell.models"
    }
  }; /* resources */
  User_settings *user_settings = (User_settings *)NULL;
#endif /* defined (MOTIF) */
  
  ENTER(create_model_dialog);
  if (cell != (struct Cell_window *)NULL)
  {
    if (cell->model_dialog != (struct Model_dialog *)NULL)
    {
      /* destroy any existing model_dialog */
      model_dialog_destroy_callback((Widget)NULL,(XtPointer)cell,
        (XtPointer)NULL);
    }
    if (MrmOpenHierarchy_base64_string(model_dialog_uidh,
      &model_dialog_hierarchy,&model_dialog_hierarchy_open))
    {
      if (ALLOCATE(cell->model_dialog,struct Model_dialog,1))
      {
        /* initialise the structure */
        cell->model_dialog->model_names = (struct Model_name *)NULL;
        cell->model_dialog->shell = (Widget)NULL;
        cell->model_dialog->window = (Widget)NULL;
        cell->model_dialog->current_model_label = (Widget)NULL;
        cell->model_dialog->membrane_list = (Widget)NULL;
        cell->model_dialog->mechanics_list = (Widget)NULL;
        cell->model_dialog->metabolism_list = (Widget)NULL;
        cell->model_dialog->coupled_list = (Widget)NULL;
        cell->model_dialog->current_model_string = (char *)NULL;
				/* make the dialog shell */
        if (cell->model_dialog->shell =
          XtVaCreatePopupShell("model_dialog_shell",
            xmDialogShellWidgetClass,cell->shell,
            XmNmwmDecorations,MWM_DECOR_ALL,
            XmNmwmFunctions,MWM_FUNC_ALL,
            XmNdeleteResponse,XmDESTROY,
            XmNtransient,FALSE,
            NULL))
        {
          /* identify the shell for the busy icon */
          create_Shell_list_item(&(cell->model_dialog->shell),
            cell->user_interface);
          /* add the destroy callback */
          XtAddCallback(cell->model_dialog->shell,XmNdestroyCallback,
            model_dialog_destroy_callback,(XtPointer)cell);
          WM_DELETE_WINDOW=XmInternAtom(XtDisplay(cell->model_dialog->shell),
            "WM_DELETE_WINDOW",FALSE);
          XmAddWMProtocolCallback(cell->model_dialog->shell,WM_DELETE_WINDOW,
            model_dialog_destroy_callback,(XtPointer)cell);
          /* register cellbacks in UIL */
          if (MrmSUCCESS == MrmRegisterNamesInHierarchy(model_dialog_hierarchy,
            callback_list,XtNumber(callback_list)))
          {
            /* set the identifier's values */
            identifier_list[0].value = (XtPointer)cell;
            identifier_list[1].value =
              (XtPointer)(cell->user_settings->text_emphasis_colour);
            /* register the identifiers */
            if (MrmSUCCESS ==
              MrmRegisterNamesInHierarchy(model_dialog_hierarchy,
                identifier_list,XtNumber(identifier_list)))
            {
              /* fetch the window widget */
              if (MrmSUCCESS == MrmFetchWidget(model_dialog_hierarchy,
                "model_dialog",cell->model_dialog->shell,
                &(cell->model_dialog->window),&model_dialog_class))
              {
                /* retrieve settings */
                if (ALLOCATE(user_settings,User_settings,1))
                {
                  XtVaGetApplicationResources(cell->model_dialog->window,
                    (XtPointer)user_settings,resources,XtNumber(resources),
                    NULL);
                  /* parse the models file, setting up the model names */
                  if (read_model_names(cell,user_settings->models_file))
                  {
                    /* add the model names to the dialog */
                    if (add_model_names(cell))
                    {
                      /* update the current model string and the current
                         model label */
                      update_current_model(cell);
                      XtManageChild(cell->model_dialog->window);
                      XtRealizeWidget(cell->model_dialog->shell);
                      return_code = 1;
                    }
                    else
                    {
                      display_message(ERROR_MESSAGE,"create_model_dialog. "
                        "Unable to add the model names to the list widgets");
                      DEALLOCATE(cell->model_dialog);
                      cell->model_dialog = (struct Model_dialog *)NULL;
                      return_code = 0;
                    }
                  }
                  else
                  {
                    display_message(ERROR_MESSAGE,"create_model_dialog. "
                      "Unable to read in the model names");
                    DEALLOCATE(cell->model_dialog);
                    cell->model_dialog = (struct Model_dialog *)NULL;
                    return_code = 0;
                  }
                }
                else
                {
                  display_message(ERROR_MESSAGE,"create_model_dialog. "
                    "Unable to allocate memory for user settings");
                  DEALLOCATE(cell->model_dialog);
                  cell->model_dialog = (struct Model_dialog *)NULL;
                  return_code = 0;
                }
              }
              else
              {
                display_message(ERROR_MESSAGE,"create_model_dialog. "
                  "Unable to fetch the window widget");
                DEALLOCATE(cell->model_dialog);
                cell->model_dialog = (struct Model_dialog *)NULL;
                return_code = 0;
              }
            }
            else
            {
              display_message(ERROR_MESSAGE,"create_model_dialog. "
                "Unable to register the identifiers");
              DEALLOCATE(cell->model_dialog);
              cell->model_dialog = (struct Model_dialog *)NULL;
              return_code = 0;  
            }
          }
          else
          {
            display_message(ERROR_MESSAGE,"create_model_dialog. "
              "Unable to register the callbacks");
            DEALLOCATE(cell->model_dialog);
            cell->model_dialog = (struct Model_dialog *)NULL;
            return_code = 0;  
          }
        }
        else
        {
        display_message(ERROR_MESSAGE,"create_model_dialog. "
          "Unable to create the dialog shell for the model dialog");
        DEALLOCATE(cell->model_dialog);
        cell->model_dialog = (struct Model_dialog *)NULL;
        return_code = 0;          
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"create_model_dialog. "
          "Unable to allocate memory for the model dialog");
        cell->model_dialog = (struct Model_dialog *)NULL;
        return_code = 0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"create_model_dialog. "
        "Unable to open the model dialog hierarchy");
      cell->model_dialog = (struct Model_dialog *)NULL;
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"create_model_dialog. "
      "Missing Cell window");
    cell->model_dialog = (struct Model_dialog *)NULL;
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END create_model_dialog() */

/*
Global functions
================
*/
int bring_up_model_dialog(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 03 February 1999

DESCRIPTION :
If there is a model dialog in existence, then bring it to the front, else
create a new one.
==============================================================================*/
{
  int return_code = 0;
  
  ENTER(bring_up_model_dialog);
  if (cell != (struct Cell_window *)NULL)
  {
    if (cell->model_dialog == (struct Model_dialog *)NULL)
    {
      /*display_message(INFORMATION_MESSAGE,"bring_up_model_dialog. "
        "creating new dialog\n");*/
      /* model dialog does not exist, so create a new one */
      if (create_model_dialog(cell))
      {
        XtPopup(cell->model_dialog->shell,XtGrabNone);
        /* set the busy cursor */
        busy_cursor_on(cell->model_dialog->shell,cell->user_interface);
        return_code = 1;
      }
      else
      {
        display_message(ERROR_MESSAGE,"bring_up_model_dialog. "
          "Error creating the model dialog");
        cell->model_dialog = (struct Model_dialog *)NULL;
        return_code = 0;
      }
    }
    else
    {
      /*display_message(INFORMATION_MESSAGE,"bring_up_model_dialog. "
        "dialog exists\n");*/
      /* dialog already exists so just pop it up */
      XtPopup(cell->model_dialog->shell,XtGrabNone);
      /* set the busy cursor */
      busy_cursor_on(cell->model_dialog->shell,cell->user_interface);
      return_code = 1;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"bring_up_model_dialog. "
      "Missing Cell window");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END bring_up_model_dialog() */

void set_model_name(struct Cell_window *cell,int type,char *name,
  struct URI *uri,char *id)
/*******************************************************************************
LAST MODIFIED : 27 February 1999

DESCRIPTION :
Adds the model <name> to the model_names array in <cell>. <type> = 1 for a
membrane model, 2 for a mechanics model, 3 for a metabolism model, and 4 for a
coupled model. <uri> is the corresponding model file. <id> is used to identify
a model specified via a node.
==============================================================================*/
{
  struct Model_name *new = (struct Model_name *)NULL;
  struct Model_name *current = (struct Model_name *)NULL;
  
  ENTER(set_model_name);
  if (cell && name && (type > 0))
  {
    if (ALLOCATE(new,struct Model_name,1))
    {
      new->name = (char *)NULL;
      new->id = (char *)NULL;
      new->uri = uri;
      new->next = (struct Model_name *)NULL;
      if (ALLOCATE(new->name,char,strlen(name)+1) &&
        ALLOCATE(new->id,char,strlen(id)+1))
      {
        sprintf(new->name,"%s\0",name);
        sprintf(new->id,"%s\0",id);
        switch (type)
        {
          case 1:
          {
            new->type = MEMBRANE_MODEL;
          } break;
          case 2:
          {
            new->type = MECHANICS_MODEL;
          } break;
          case 3:
          {
            new->type = METABOLISM_MODEL;
          } break;
          case 4:
          {
            new->type = COUPLED_MODEL;
          } break;
          default:
          {
            new->type = UNKNOWN_MODEL;
          } break;
        } /* switch (type) */
        if (cell->model_dialog->model_names == (struct Model_name *)NULL)
        {
          cell->model_dialog->model_names = new;
        }
        else
        {
          current = cell->model_dialog->model_names;
          while (current->next != (struct Model_name *)NULL)
          {
            current = current->next;
          } /* while (current != (struct Model_name *)NULL) */
          current->next = new;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"set_model_name. "
          "Unable to allocate memory for the name %s",name);
        DEALLOCATE(new);
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"set_model_name. "
        "Unable to allocate memory for the model %s",name);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"set_model_name. "
      "Invalid arguments");
  }
  LEAVE;
} /* END set_model_name() */

void close_model_dialog(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 16 September 1999

DESCRIPTION :
If there is a model dialog in existence, destroy it.
==============================================================================*/
{
  ENTER(close_model_dialog);
  model_dialog_destroy_callback((Widget)NULL,(XtPointer)cell,
    (XtPointer)NULL);
  LEAVE;
} /* END close_model_dialog() */
