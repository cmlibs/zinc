/*******************************************************************************
FILE : cell_window.c

LAST MODIFIED : 01 February 2001

DESCRIPTION :
The Cell Interface main window.
==============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Xm/Command.h>
#include <Xm/DialogS.h>
#include <Xm/MwmUtil.h>
#include <Xm/List.h>
#include <Xm/Protocols.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include "cell/cell_window.h"
#include "cell/cell_window.uidh"
#include "user_interface/filedir.h"
#include "user_interface/gui_dialog_macros.h"

/*
Module types
============
*/
struct Cell_window
/*******************************************************************************
LAST MODIFIED : 31 January 2001

DESCRIPTION :
The Cell Interface main window structure.
==============================================================================*/
{
  /* The main cell interface object */
  struct Cell_interface *interface;
  /* The user interface */
  struct User_interface *user_interface;
  /* The main shell and window widgets */
  Widget shell;
  Widget window;
  /* The activation widget for UnEmap */
  Widget unemap_activation_button;
  /* The distributed editing toggle */
  Widget distributed_toggle;
  /* The form to hold the 3D scene */
  Widget scene_form;
  /* The toolbar form */
  Widget toolbar_form;
  /* The export to CMISS button */
  Widget export_button;
}; /* struct Cell_window */

/*
Module variables
================
*/
static int cell_window_hierarchy_open=0;
static MrmHierarchy cell_window_hierarchy;

/*
Module functions
================
*/
DECLARE_DIALOG_IDENTIFY_FUNCTION(cell_window,Cell_window, \
  unemap_activation_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(cell_window,Cell_window, \
  distributed_toggle)
DECLARE_DIALOG_IDENTIFY_FUNCTION(cell_window,Cell_window, \
  scene_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(cell_window,Cell_window, \
  toolbar_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(cell_window,Cell_window, \
  export_button)

static void close_model_cb(Widget widget,XtPointer cell_window_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 19 October 2000

DESCRIPTION :
Called when the "Close" option is chosen under the file menu.
==============================================================================*/
{
  struct Cell_window *cell_window;
  struct Cell_interface *cell_interface;

  ENTER(close_model_cb);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
  if ((cell_window = (struct Cell_window *)cell_window_void) &&
    (cell_interface = cell_window->interface))
  {
    Cell_interface_close_model(cell_interface);
  }
  else
  {
    display_message(ERROR_MESSAGE,"close_model_cb. "
      "Missing cell window");
  }
  LEAVE;
} /* close_model_cb() */

static void variables_button_cb(Widget widget,XtPointer cell_window_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 19 October 2000

DESCRIPTION :
Activation callback for the "Variables" button in the main cell menu.
==============================================================================*/
{
  struct Cell_window *cell_window;

  ENTER(variables_button_cb);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
  if (cell_window = (struct Cell_window *)cell_window_void)
  {
    Cell_interface_edit_component_variables(cell_window->interface,
      /*name*/(char *)NULL,/*reset*/1);
  }
  else
  {
    display_message(ERROR_MESSAGE,"variable_button_cb. "
      "Missing cell window");
  }
  LEAVE;
} /* variables_button_cb() */

#if defined (CELL_DISTRIBUTED)
static void export_button_cb(Widget widget,XtPointer cell_window_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 31 January 2001

DESCRIPTION :
Activation callback for the "Export to CMISS..." button in the file menu.
==============================================================================*/
{
  struct Cell_window *cell_window;

  ENTER(export_button_cb);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
  if (cell_window = (struct Cell_window *)cell_window_void)
  {
    Cell_interface_pop_up_export_dialog(cell_window->interface);
  }
  else
  {
    display_message(ERROR_MESSAGE,"export_button_cb. "
      "Missing cell window");
  }
  LEAVE;
} /* export_button_cb() */
#endif /* defined (CELL_DISTRIBUTED) */

static void show_unemap_button_cb(Widget widget,XtPointer cell_window_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 05 November 2000

DESCRIPTION :
Activation callback for the "Show" option in the UnEmap menu in the main
cell menu.
==============================================================================*/
{
  struct Cell_window *cell_window;

  ENTER(show_unemap_button_cb);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
  if (cell_window = (struct Cell_window *)cell_window_void)
  {
    Cell_interface_pop_up_unemap(cell_window->interface);
  }
  else
  {
    display_message(ERROR_MESSAGE,"show_unemap_button_cb. "
      "Missing cell window");
  }
  LEAVE;
} /* show_unemap_button_cb() */

static void clear_unemap_button_cb(Widget widget,XtPointer cell_window_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 05 November 2000

DESCRIPTION :
Activation callback for the "Clear" option in the UnEmap menu in the main
cell menu.
==============================================================================*/
{
  struct Cell_window *cell_window;

  ENTER(clear_unemap_button_cb);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
  if (cell_window = (struct Cell_window *)cell_window_void)
  {
    Cell_interface_clear_unemap(cell_window->interface);
  }
  else
  {
    display_message(ERROR_MESSAGE,"clear_unemap_button_cb. "
      "Missing cell window");
  }
  LEAVE;
} /* clear_unemap_button_cb() */

static void save_signals_toggle_changed_cb(Widget widget,
  XtPointer cell_window_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 05 November 2000

DESCRIPTION :
Activation callback for the "Save signals" toggle in the UnEmap menu in the main
cell menu.
==============================================================================*/
{
  struct Cell_window *cell_window;
  Boolean state;

  ENTER(save_signals_toggle_changed_cb);
	USE_PARAMETER(call_data);
  if (cell_window = (struct Cell_window *)cell_window_void)
  {
    XtVaGetValues(widget,
      XmNset,&state,
      NULL);
    if (state)
    {
      Cell_interface_set_save_signals(cell_window->interface,1);
    }
    else
    {
      Cell_interface_set_save_signals(cell_window->interface,0);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"save_signals_toggle_changed_cb. "
      "Missing cell window");
  }
  LEAVE;
} /* save_signals_toggle_changed_cb() */

static void calculate_button_cb(Widget widget,XtPointer cell_window_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Activation callback for the "Calculate" button in the main cell menu.
==============================================================================*/
{
  struct Cell_window *cell_window;

  ENTER(calculate_button_cb);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
  if (cell_window = (struct Cell_window *)cell_window_void)
  {
    Cell_interface_pop_up_calculate_dialog(cell_window->interface);
  }
  else
  {
    display_message(ERROR_MESSAGE,"calculate_button_cb. "
      "Missing cell window");
  }
  LEAVE;
} /* calculate_button_cb() */

static void show_scene_cb(Widget widget,XtPointer cell_window_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Value changed callback for the "Show scene" toggle button.
==============================================================================*/
{
  struct Cell_window *cell_window;
  Boolean set;
  static Dimension shell_width,shell_height;
  Dimension scene_width,scene_height,toolbar_height;
  Dimension width1,height1,width2,height2,width,height;
  Widget *list1,*list2;

  ENTER(show_scene_cb);
	USE_PARAMETER(call_data);
  if ((cell_window = (struct Cell_window *)cell_window_void) &&
    cell_window->shell)
  {
    XtVaGetValues(widget,
      XmNset,&set,
      NULL);
    if (set)
    {
      /* Get the original shell size - should always be a set called before
         an unset ?!? */
      XtVaGetValues(cell_window->shell,
        XmNwidth,&shell_width,
        XmNheight,&shell_height,
        NULL);
      width = shell_width;
      height = shell_height;
      if (cell_window->scene_form && cell_window->toolbar_form)
      {
        if (!XtIsManaged(cell_window->scene_form))
        {
          XtManageChild(cell_window->toolbar_form);
          XtManageChild(cell_window->scene_form);
          /* Resize the window */
          XtVaGetValues(cell_window->scene_form,
            XmNwidth,&width1,
            XmNheight,&height1,
            NULL);
          XtVaGetValues(cell_window->toolbar_form,
            XmNwidth,&width2,
            XmNheight,&height2,
            NULL);
          /* Set the height of the toolbar form - from the height of one of
             the radio buttons */
          XtVaGetValues(cell_window->toolbar_form,
            XmNchildren,&list1,
            NULL);
          XtVaGetValues(list1[0],
            XmNchildren,&list2,
            NULL);
          XtVaGetValues(list2[0],
            XmNheight,&toolbar_height,
            NULL);
          if (toolbar_height > height2)
          {
            height2 = toolbar_height;
            XtVaSetValues(cell_window->toolbar_form,
              XmNheight,toolbar_height,
              NULL);
          }
          /* Check that the actual scene is visable */
          if (height1 < 5)
          {
            height1 = width1;
            XtVaSetValues(cell_window->scene_form,
              XmNheight,height1,
              NULL);
          }
          scene_width = (width1 > width2) ? width1 : width2;
          scene_height = height1 + height2;
          if (scene_width > width)
          {
            width = scene_width;
          }
          height += scene_height;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"show_scene_cb. "
          "Missing scene fomrs for the cell window");
      }
    }
    else
    {
      width = shell_width;
      height = shell_height;
      if (cell_window->scene_form && cell_window->toolbar_form)
      {
        if (XtIsManaged(cell_window->scene_form))
        {
          XtUnmanageChild(cell_window->scene_form);
          XtUnmanageChild(cell_window->toolbar_form);
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"show_scene_cb. "
          "Missing scene forms for the cell window");
      }
    }
    XtVaSetValues(cell_window->shell,
      XmNwidth,width,
      XmNheight,height,
      NULL);    
  }
  else
  {
    display_message(ERROR_MESSAGE,"show_scene_cb. "
      "Missing cell window");
  }
  LEAVE;
} /* show_scene_cb() */

static void distributed_cb(Widget widget,XtPointer cell_window_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 January 2001

DESCRIPTION :
Value changed callback for the "Distributed" toggle button.
==============================================================================*/
{
#if defined (CELL_DISTRIBUTED)
  struct Cell_window *cell_window;
  Boolean set;
  int return_code;

  ENTER(distributed_cb);
	USE_PARAMETER(call_data);
  if (cell_window = (struct Cell_window *)cell_window_void)
  {
    /* Get the state of the toggle */
    XtVaGetValues(widget,
      XmNset,&set,
      NULL);
    if (set)
    {
      if (return_code = Cell_interface_pop_up_distributed_editing_dialog(
        cell_window->interface,widget))
      {
        XtSetSensitive(cell_window->export_button,True);
      }
    }
    else
    {
      return_code = Cell_interface_destroy_distributed_editing_dialog(
        cell_window->interface);
      XtSetSensitive(cell_window->export_button,False);
    }
    /* If an error occurred, reset the toggle */
    if (!return_code)
    {
      XtVaSetValues(widget,
      XmNset,!set,
      NULL);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"distributed_cb. "
      "Missing cell window");
  }
  LEAVE;
#else /* defined (CELL_DISTRIBUTED) */
  ENTER(distributed_cb);
  USE_PARAMETER(widget);
  USE_PARAMETER(cell_window_void);
  USE_PARAMETER(call_data);
  LEAVE;
#endif /* defined (CELL_DISTRIBUTED) */
} /* distributed_cb() */

static int model_file_open_cb(char *filename,XtPointer cell_window_void)
/*******************************************************************************
LAST MODIFIED : 19 October 2000

DESCRIPTION :
Callback function for opening a model file via the File -> Open menu
==============================================================================*/
{
  int return_code = 0;
  struct Cell_window *cell_window = (struct Cell_window *)NULL;

  ENTER(model_file_open_cb);
  if (cell_window = (struct Cell_window *)cell_window_void)
  {
    return_code = Cell_interface_read_model(cell_window->interface,filename);
  }
  else
  {
    display_message(ERROR_MESSAGE,"model_file_open_cb. "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* model_file_open_cb() */

static int model_file_save_cb(char *filename,XtPointer cell_window_void)
/*******************************************************************************
LAST MODIFIED : 13 November 2000

DESCRIPTION :
Callback function for saving a model file via the File -> Save menu
==============================================================================*/
{
  int return_code = 0;
  struct Cell_window *cell_window = (struct Cell_window *)NULL;

  ENTER(model_file_save_cb);
  if (cell_window = (struct Cell_window *)cell_window_void)
  {
    return_code = Cell_interface_write_model(cell_window->interface,filename);
  }
  else
  {
    display_message(ERROR_MESSAGE,"model_file_save_cb. "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* model_file_save_cb() */

static int ipcell_file_save_cb(char *filename,XtPointer cell_window_void)
/*******************************************************************************
LAST MODIFIED : 11 January 2001

DESCRIPTION :
Callback function for saving a ipcell file via the File -> Save menu
==============================================================================*/
{
  int return_code = 0;
  struct Cell_window *cell_window = (struct Cell_window *)NULL;

  ENTER(ipcell_file_save_cb);
  if (cell_window = (struct Cell_window *)cell_window_void)
  {
    return_code =
      Cell_interface_write_model_to_ipcell_file(cell_window->interface,
      filename);
  }
  else
  {
    display_message(ERROR_MESSAGE,"ipcell_file_save_cb. "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* ipcell_file_save_cb() */

/*
Global functions
================
*/
struct Cell_window *CREATE(Cell_window)(
  struct Cell_interface *cell_interface,
  struct User_interface *user_interface,
  XtCallbackProc exit_callback)
/*******************************************************************************
LAST MODIFIED : 31 January 2001

DESCRIPTION :
Creates the Cell interface main window.
==============================================================================*/
{
  Atom WM_DELETE_WINDOW;
	MrmType cell_window_class;
  struct Cell_window *cell_window;
  struct File_open_data *model_file_open_data;
  struct File_open_data *model_file_save_data;
  struct File_open_data *ipcell_file_save_data;
  static MrmRegisterArg callback_list[] = {
    {"exit_callback",(XtPointer)NULL},
    {"close_model_callback",(XtPointer)close_model_cb},
    {"file_open_and_read",(XtPointer)open_file_and_read},
    {"file_open_and_write",(XtPointer)open_file_and_write},
    {"variables_button_callback",(XtPointer)variables_button_cb},
    {"calculate_button_callback",(XtPointer)calculate_button_cb},
    {"show_scene_callback",(XtPointer)show_scene_cb},
    {"distributed_callback",(XtPointer)distributed_cb},
    {"show_unemap_button_callback",(XtPointer)show_unemap_button_cb},
    {"clear_unemap_button_callback",(XtPointer)clear_unemap_button_cb},
    {"save_signals_toggle_changed_cb",
     (XtPointer)save_signals_toggle_changed_cb},
    {"cell_window_id_unemap_button",
     (XtPointer)DIALOG_IDENTIFY(cell_window,unemap_activation_button)},
    {"id_distributed_toggle",
     (XtPointer)DIALOG_IDENTIFY(cell_window,distributed_toggle)},
    {"cell_window_id_3d_form",
     (XtPointer)DIALOG_IDENTIFY(cell_window,scene_form)},
    {"cell_window_id_3d_toolbar",
     (XtPointer)DIALOG_IDENTIFY(cell_window,toolbar_form)},
    {"id_export_button",
     (XtPointer)DIALOG_IDENTIFY(cell_window,export_button)}
  }; /* callback_list */
  static MrmRegisterArg identifier_list[] = {
    {"cell_window_structure",(XtPointer)NULL},
    {"model_file_open_data",(XtPointer)NULL},
    {"model_file_save_data",(XtPointer)NULL},
    {"ipcell_file_save_data",(XtPointer)NULL}
  }; /* identifier_list */
  
  ENTER(CREATE(Cell_window));
  if (cell_interface && user_interface)
  {
    if (MrmOpenHierarchy_base64_string(cell_window_uidh,&cell_window_hierarchy,
			&cell_window_hierarchy_open))
    {
      if (ALLOCATE(cell_window,struct Cell_window,1))
      {
        cell_window->interface = cell_interface;
        cell_window->user_interface = user_interface;
        cell_window->window = (Widget)NULL;
        cell_window->shell = (Widget)NULL;
        cell_window->unemap_activation_button = (Widget)NULL;
        cell_window->scene_form = (Widget)NULL;
        cell_window->toolbar_form = (Widget)NULL;
        cell_window->export_button = (Widget)NULL;
        if (cell_window->shell = XtVaCreatePopupShell("cell_window_shell",
          xmDialogShellWidgetClass,user_interface->application_shell,
					XmNdeleteResponse,XmDO_NOTHING,
          XmNmwmDecorations,MWM_DECOR_ALL,
          XmNmwmFunctions,MWM_FUNC_ALL,
          XmNtransient,FALSE,
          NULL))
        {
          /* identify the shell for the busy icon */
          create_Shell_list_item(&(cell_window->shell),user_interface);
          /* add the destroy callback */
          XtAddCallback(cell_window->shell,XmNdestroyCallback,exit_callback,
            (XtPointer)cell_window);
          WM_DELETE_WINDOW=XmInternAtom(XtDisplay(cell_window->shell),
            "WM_DELETE_WINDOW",FALSE);
          XmAddWMProtocolCallback(cell_window->shell,WM_DELETE_WINDOW,
            exit_callback,(XtPointer)cell_window);
          /* register cellbacks in UIL */
          callback_list[0].value = (XtPointer)exit_callback;
          if (MrmSUCCESS == MrmRegisterNamesInHierarchy(cell_window_hierarchy,
            callback_list,XtNumber(callback_list)))
          {
            /* set the identifier's values */
            /* the cell window object */
            identifier_list[0].value = (XtPointer)cell_window;
            /* the model file open data object */
            if (model_file_open_data = create_File_open_data(".cell.xml",
              REGULAR,model_file_open_cb,(XtPointer)cell_window,0,
              user_interface))
            {
              identifier_list[1].value = (XtPointer)model_file_open_data;
            }
            else
            {
              identifier_list[1].value = (XtPointer)NULL;
            }
            /* the model file save data object */
            if (model_file_save_data = create_File_open_data(".cell.xml",
              REGULAR,model_file_save_cb,(XtPointer)cell_window,0,
              user_interface))
            {
              identifier_list[2].value = (XtPointer)model_file_save_data;
            }
            else
            {
              identifier_list[2].value = (XtPointer)NULL;
            }
            /* the ipcell file save data object */
            if (ipcell_file_save_data = create_File_open_data(".ipcell",
              REGULAR,ipcell_file_save_cb,(XtPointer)cell_window,0,
              user_interface))
            {
              identifier_list[3].value = (XtPointer)ipcell_file_save_data;
            }
            else
            {
              identifier_list[3].value = (XtPointer)NULL;
            }
            /* register the identifiers */
            if (MrmSUCCESS == MrmRegisterNamesInHierarchy(cell_window_hierarchy,
              identifier_list,XtNumber(identifier_list)))
            {
              /* fetch the window widget */
              if (MrmSUCCESS == MrmFetchWidget(cell_window_hierarchy,
                "cell_window",cell_window->shell,&(cell_window->window),
                &cell_window_class))
              {
                /* Only want the scene to be visable when required ?? */
                XtUnmanageChild(cell_window->toolbar_form);
                XtUnmanageChild(cell_window->scene_form);
                /* Only enable the distributed toggle when in distributed
                 * mode */
#if defined (CELL_DISTRIBUTED)
                if (cell_window->distributed_toggle)
                {
                  XtSetSensitive(cell_window->distributed_toggle,True);
                }
#endif /* defined (CELL_DISTRIBUTED) */
                /* If in the distributed mode, need to add the callback for the
                 * export button, otherwise remove the button from the menu
                 */
                if (cell_window->export_button)
                {
                  /* Always start with the export button insensitive */
                  XtSetSensitive(cell_window->export_button,False);
#if defined (CELL_DISTRIBUTED)
                  XtAddCallback(cell_window->export_button,XmNactivateCallback,
                    export_button_cb,(XtPointer)cell_window);
#endif /* defined (CELL_DISTRIBUTED) */
                }
                /* Manage the cell window */
								XtManageChild(cell_window->window);
                XtRealizeWidget(cell_window->shell);
              }
              else
              {
                display_message(ERROR_MESSAGE,"CREATE(Cell_window). "
                  "Unable to fetch the cell window's window widget");
                DESTROY(Cell_window)(&cell_window);
                cell_window = (struct Cell_window *)NULL;
              }
            }
            else
            {
              display_message(ERROR_MESSAGE,"CREATE(Cell_window). "
                "Unable to register the identifiers");
              DESTROY(Cell_window)(&cell_window);
              cell_window = (struct Cell_window *)NULL;
            }
          }
          else
          {
            display_message(ERROR_MESSAGE,"CREATE(Cell_window). "
              "Unable to register the callbacks");
            DESTROY(Cell_window)(&cell_window);
            cell_window = (struct Cell_window *)NULL;
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"CREATE(Cell_window). "
            "Unable to create the cell window shell widget");
          DESTROY(Cell_window)(&cell_window);
          cell_window = (struct Cell_window *)NULL;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"CREATE(Cell_window). "
          "Unable to allocate memory for the cell window object");
        cell_window = (struct Cell_window *)NULL;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"CREATE(Cell_window). "
        "Unable to open the cell window hierarchy");
      cell_window = (struct Cell_window *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Cell_window). "
      "Invalid argument(s)");
    cell_window = (struct Cell_window *)NULL;
  }
  LEAVE;
  return(cell_window);
} /* CREATE(Cell_window)() */

int DESTROY(Cell_window)(struct Cell_window **cell_window_address)
/*******************************************************************************
LAST MODIFIED : 18 October 2000

DESCRIPTION :
Destroys the Cell_window object.
==============================================================================*/
{
  int return_code;
  struct Cell_window *cell_window;
  ENTER(DESTROY(Cell_window));
  if (cell_window_address && (cell_window = *cell_window_address))
	{
    /* make sure the window isn't currently being displayed */
    if (cell_window->shell)
    {
      XtPopdown(cell_window->shell);
    }
		DEALLOCATE(*cell_window_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Cell_window).  Invalid argument(s)");
		return_code=0;
	}
  LEAVE;
  return(return_code);
} /* DESTROY(Cell_window)() */

void Cell_window_pop_up(struct Cell_window *cell_window)
/*******************************************************************************
LAST MODIFIED : 18 October 2000

DESCRIPTION :
Brings up the <cell_window>
==============================================================================*/
{
  ENTER(Cell_window_pop_up);
  if (cell_window && cell_window->shell)
  {
    XtPopup(cell_window->shell,XtGrabNone);
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_window_pop_up. "
      "Invalid argument(s)");
  }
  LEAVE;
} /* Cell_window_pop_up() */

int Cell_window_set_title_bar(struct Cell_window *cell_window,char *title)
/*******************************************************************************
LAST MODIFIED : 19 October 2000

DESCRIPTION :
Sets the main windows <title>
==============================================================================*/
{
  int return_code = 0;
  
  ENTER(Cell_window_set_title_bar);
  if (cell_window && cell_window->window)
  {
    XtVaSetValues(cell_window->window,
      XmNdialogTitle,XmStringCreateSimple(title),
      NULL);
    return_code = 1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_window_set_title_bar. "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_window_set_title_bar() */

Widget Cell_window_get_shell(struct Cell_window *cell_window)
/*******************************************************************************
LAST MODIFIED : 31 October 2000

DESCRIPTION :
Returns the shell widget from the given <cell_window>
==============================================================================*/
{
  Widget shell;
  
  ENTER(Cell_window_get_shell);
  if (cell_window)
  {
    shell = cell_window->shell;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_window_get_shell. "
      "Invalid argument(s)");
    shell = (Widget)NULL;
  }
  LEAVE;
  return(shell);
} /* Cell_window_get_shell() */

Widget Cell_window_get_window(struct Cell_window *cell_window)
/*******************************************************************************
LAST MODIFIED : 31 October 2000

DESCRIPTION :
Returns the window widget from the given <cell_window>
==============================================================================*/
{
  Widget window;
  
  ENTER(Cell_window_get_window);
  if (cell_window)
  {
    window = cell_window->window;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_window_get_window. "
      "Invalid argument(s)");
    window = (Widget)NULL;
  }
  LEAVE;
  return(window);
} /* Cell_window_get_window() */

Widget Cell_window_get_unemap_activation_button(struct Cell_window *cell_window)
/*******************************************************************************
LAST MODIFIED : 01 November 2000

DESCRIPTION :
Returns the UnEmap activation widget from the given <cell_window>
==============================================================================*/
{
  Widget unemap_activation_button;
  
  ENTER(Cell_window_get_unemap_activation_button);
  if (cell_window)
  {
    unemap_activation_button = cell_window->unemap_activation_button;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_window_get_unemap_activation_button. "
      "Invalid argument(s)");
    unemap_activation_button = (Widget)NULL;
  }
  LEAVE;
  return(unemap_activation_button);
} /* Cell_window_get_unemap_activation_button() */

Widget Cell_window_get_root_shell(struct Cell_window *cell_window)
/*******************************************************************************
LAST MODIFIED : 07 November 2000

DESCRIPTION :
Returns the root shell widget from the given <cell_window>
==============================================================================*/
{
  Widget root_shell;
  
  ENTER(Cell_window_get_root_shell);
  if (cell_window)
  {
    root_shell = cell_window->shell;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_window_get_root_shell. "
      "Invalid argument(s)");
    root_shell = (Widget)NULL;
  }
  LEAVE;
  return(root_shell);
} /* Cell_window_get_root_shell() */

Widget Cell_window_get_scene_form(struct Cell_window *cell_window)
/*******************************************************************************
LAST MODIFIED : 17 November 2000

DESCRIPTION :
Returns the scene form widget from the given <cell_window>
==============================================================================*/
{
  Widget scene_form;
  
  ENTER(Cell_window_get_scene_form);
  if (cell_window)
  {
    scene_form = cell_window->scene_form;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_window_get_scene_form. "
      "Invalid argument(s)");
    scene_form = (Widget)NULL;
  }
  LEAVE;
  return(scene_form);
} /* Cell_window_get_scene_form() */

Widget Cell_window_get_toolbar_form(struct Cell_window *cell_window)
/*******************************************************************************
LAST MODIFIED : 17 November 2000

DESCRIPTION :
Returns the toolbar form widget from the given <cell_window>
==============================================================================*/
{
  Widget toolbar_form;
  
  ENTER(Cell_window_get_toolbar_form);
  if (cell_window)
  {
    toolbar_form = cell_window->toolbar_form;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_window_get_toolbar_form. "
      "Invalid argument(s)");
    toolbar_form = (Widget)NULL;
  }
  LEAVE;
  return(toolbar_form);
} /* Cell_window_get_toolbar_form() */

void exit_cell_window(Widget widget,XtPointer cell_window_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Called when the Exit function is selected from the window manager menu in the
Cell window - used in stand-alone Cell
==============================================================================*/
{
  struct Cell_window *cell_window;
  struct Cell_interface *cell_interface;

  ENTER(exit_cell_window);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
  if ((cell_window = (struct Cell_window *)cell_window_void) &&
    (cell_interface = cell_window->interface))
  {
    if (cell_window->shell)
    {
      XtPopdown(cell_window->shell);
    }
    DESTROY(Cell_interface)(&cell_interface);
    exit(0);
  }
  else
  {
    display_message(ERROR_MESSAGE,"exit_cell_window. "
      "Missing cell window");
  }
  LEAVE;
} /* exit_cell_window() */

void close_cell_window(Widget widget,XtPointer cell_window_void,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Called when the Exit function is selected from the window manager menu in the
Cell window - used when Cell is part of CMGUI
==============================================================================*/
{
  struct Cell_window *cell_window;

  ENTER(close_cell_window);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
  if (cell_window = (struct Cell_window *)cell_window_void)
  {
    Cell_interface_close(cell_window->interface);
    if (cell_window->shell)
    {
      XtPopdown(cell_window->shell);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"close_cell_window. "
      "Missing cell window");
  }
  LEAVE;
} /* close_cell_window() */

