/*******************************************************************************
FILE : cell_control_curve.c

LAST MODIFIED : 28 August 2000

DESCRIPTION :
Functions for CELL to interact with CMGUI control_curves.
==============================================================================*/
#if defined (MOTIF)
#include <string.h>
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Xm/MwmUtil.h>
#include <Xm/DialogS.h>
#include <Xm/Protocols.h>
#include <Xm/ToggleB.h>
#endif /* if defined (MOTIF) */
#include <stdio.h>
#include "cell/cell_window.h"
#include "cell/cell_parameter.h"
#include "cell/cell_variable.h"
#include "cell/cell_control_curve.h"
#include "cell/export_control_curves_dialog.uidh"
#include "cell/output.h"
#include "user_interface/filedir.h"
/*
Local types
===========
*/

/*
Local variables
===============
*/
#if defined (MOTIF)
static int export_control_curves_dialog_hierarchy_open=0;
static MrmHierarchy export_control_curves_dialog_hierarchy;
#endif /* defined (MOTIF) */

/*
Local functions
===============
*/
static void export_control_curves_dialog_destroy_callback(Widget widget,
  XtPointer cell_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 September 1999

DESCRIPTION :
Callback for when the export time variables dialog is destroy via the 
window manager menu ??
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(export_control_curves_dialog_destroy_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (cell->export_control_curve_dialog)
    {
      if (cell->export_control_curve_dialog->shell)
      {
        /* remove the shell from the shell list */
        destroy_Shell_list_item_from_shell(
					&(cell->export_control_curve_dialog->shell),cell->user_interface);
        /* make sure the dialog is no longer visible */
        XtPopdown(cell->export_control_curve_dialog->shell);
        /* Unmanage the shell */
        XtUnmanageChild(cell->export_control_curve_dialog->shell);
      }
      DEALLOCATE(cell->export_control_curve_dialog);
      cell->export_control_curve_dialog = 
				(struct Export_control_curve_dialog *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,
			"export_control_curves_dialog_destroy_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END export_control_curves_dialog_destroy_callback() */

int file_open_iptime_callback(char *filename,
	XtPointer export_control_curve_dialog)
/*******************************************************************************
LAST MODIFIED : 28 August 2000

DESCRIPTION :
Callback for the file selection dialog for the file in the export time 
variables dialog.
==============================================================================*/
{
  int return_code = 0;
  struct Export_control_curve_dialog *export = 
		(struct Export_control_curve_dialog *)NULL;

  ENTER(file_open_callback);
  if (export = (struct Export_control_curve_dialog *)
		export_control_curve_dialog)
  {
    /* set the label widget */
    if (export->file_label)
    {
      XtVaSetValues(export->file_label,
        XmNlabelString,XmStringCreateSimple(filename),
				NULL);
    }
    /* and save the file name */
    if (export->file_name)
    {
      DEALLOCATE(export->file_name);
    }
    if (ALLOCATE(export->file_name,char,strlen(filename)+1))
		{
			strcpy(export->file_name,filename);
      return_code = 1;
    }
    else
    {
      display_message(ERROR_MESSAGE,"file_open_callback. "
        "Unable to allocate memory for the file name");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"file_open_callback. "
      "Missing export dialog");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END file_open_callback() */

static void identify_variables_rowcol(Widget widget,
  XtPointer cell_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 September 1999

DESCRIPTION :
Identifies the rowcolumn for the list of time variables.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(identify_variables_rowcol);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    cell->export_control_curve_dialog->variables_rowcol = widget;
  }
  else
  {
    display_message(ERROR_MESSAGE,"identify_variables_rowcol. "
      "Missing cell window");
  }
  LEAVE;
} /* END identify_variables_rowcol() */

static void identify_file_label(Widget widget,
  XtPointer cell_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 September 1999

DESCRIPTION :
Identifies the label widget for the file name
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(identify_file_label);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    cell->export_control_curve_dialog->file_label = widget;
  }
  else
  {
    display_message(ERROR_MESSAGE,"identify_file_label. "
      "Missing cell window");
  }
  LEAVE;
} /* END identify_file_label() */

static void browse_callback(Widget widget,
  XtPointer cell_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 September 1999

DESCRIPTION :
Callback for the browse button for the file.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  struct File_open_data *file_open_data;
  
  ENTER(browse_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (file_open_data = create_File_open_data(".iptime",REGULAR,
      file_open_iptime_callback,(XtPointer)(cell->export_control_curve_dialog),
			0,cell->user_interface))
    {
      open_file_and_write(widget,(XtPointer)file_open_data,call_data);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"browse_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END browse_callback() */

static void ok_button_callback(Widget widget,
  XtPointer cell_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 17 September 1999

DESCRIPTION :
Callback for the OK button in the export time variable dialog.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;

  ENTER(ok_button_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    /* write out all the selected control curves */
    export_control_curves_to_file(cell->export_control_curve_dialog);
		/* destroy the dialog */
		export_control_curves_dialog_destroy_callback((Widget)NULL,
			(XtPointer)cell,(XtPointer)NULL);
  }
  else
  {
    display_message(ERROR_MESSAGE,"ok_button_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END ok_button_callback() */

static void cancel_button_callback(Widget widget,
  XtPointer cell_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 September 1999

DESCRIPTION :
Callback for the cancel button in the export time variable dialog.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;

  ENTER(cancel_button_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
		/* destroy the dialog */
		export_control_curves_dialog_destroy_callback((Widget)NULL,
			(XtPointer)cell,(XtPointer)NULL);
  }
  else
  {
    display_message(ERROR_MESSAGE,"cancel_button_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END cancel_button_callback() */

static int create_control_curve_toggle(struct Control_curve *variable,
	void *dialog_void)
/*******************************************************************************
LAST MODIFIED : 15 June 2000

DESCRIPTION :
control_curve iterator function for creating all the time variable toggles.
==============================================================================*/
{
	int return_code;
	char *name;
  struct Export_control_curve_dialog *dialog;
	
	ENTER(create_control_curve_toggle);
	if ((dialog = (struct Export_control_curve_dialog *)dialog_void) &&
    dialog->variables_rowcol)
	{
		GET_NAME(Control_curve)(variable,&name);
    dialog->number_of_curves++;
    if (dialog->number_of_curves == 1)
    {
      ALLOCATE(dialog->curve_toggles,Widget,dialog->number_of_curves);
    }
    else
    {
      REALLOCATE(dialog->curve_toggles,dialog->curve_toggles,Widget,
        dialog->number_of_curves);
    }
    if (dialog->curve_toggles)
    {
      dialog->curve_toggles[dialog->number_of_curves-1] =
        XtVaCreateManagedWidget("cell_control_curve_toggle",
          xmToggleButtonWidgetClass,dialog->variables_rowcol,
          XmNlabelString,XmStringCreateSimple(name),
          NULL);
      return_code = 1;
    }
    else
    {
      dialog->number_of_curves--;
      return_code = 0;
    }
	}
	else
	{
		return_code = 0;
	}
	LEAVE;
	return(return_code);
} /* END create_control_curve_toggle() */

static int create_control_curve_toggles(
  struct Export_control_curve_dialog *dialog,
	struct MANAGER(Control_curve) *control_curve_manager)
/*******************************************************************************
LAST MODIFIED : 15 June 2000

DESCRIPTION :
Creates the toggle button widgets for all of the currently managed time 
variables.
==============================================================================*/
{
	int return_code = 0;

	ENTER(create_control_curve_toggles);
	return_code = FOR_EACH_OBJECT_IN_MANAGER(Control_curve)(
		create_control_curve_toggle,(void *)dialog,control_curve_manager);
	LEAVE;
	return(return_code);
} /* END create_control_curve_toggles() */

static int create_export_control_curves_dialog(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 28 August 2000

DESCRIPTION :
Create a new export dialog.
==============================================================================*/
{
  int return_code = 0;
  Atom WM_DELETE_WINDOW;
  MrmType export_control_curves_dialog_class;
  static MrmRegisterArg callback_list[] = {
    {"identify_variables_rowcol",(XtPointer)identify_variables_rowcol},
    {"identify_file_label",(XtPointer)identify_file_label},
    {"browse_callback",(XtPointer)browse_callback},
    {"ok_button_callback",(XtPointer)ok_button_callback},
    {"cancel_button_callback",(XtPointer)cancel_button_callback}
  }; /* callback_list */
  static MrmRegisterArg identifier_list[] = {
    {"cell_window_structure",(XtPointer)NULL},
    {"window_width",(XtPointer)NULL},
    {"window_height",(XtPointer)NULL},
    {"default_file_name",(XtPointer)NULL}
  }; /* identifier_list */
  unsigned int width,height;
  
  ENTER(create_export_control_curves_dialog);
  if (cell != (struct Cell_window *)NULL)
  {
    if (cell->export_control_curve_dialog != 
			(struct Export_control_curve_dialog *)NULL)
    {
      /* destroy any existing variables_dialog */
      export_control_curves_dialog_destroy_callback((Widget)NULL,
				(XtPointer)cell,(XtPointer)NULL);
    }
    if (MrmOpenHierarchy_base64_string(export_control_curves_dialog_uidh,
      &export_control_curves_dialog_hierarchy,
			&export_control_curves_dialog_hierarchy_open))
    {
      if (ALLOCATE(cell->export_control_curve_dialog,
				struct Export_control_curve_dialog,1))
      {
        /* initialise the structure */
        cell->export_control_curve_dialog->shell = (Widget)NULL;
        cell->export_control_curve_dialog->window = (Widget)NULL;
				cell->export_control_curve_dialog->variables_rowcol = (Widget)NULL;
				cell->export_control_curve_dialog->file_label = (Widget)NULL;
				cell->export_control_curve_dialog->file_name = (char *)NULL;
        cell->export_control_curve_dialog->curve_toggles = (Widget *)NULL;
        cell->export_control_curve_dialog->number_of_curves = 0;
        cell->export_control_curve_dialog->control_curve_manager =
          (cell->control_curve).control_curve_manager;
				/* set the default file names */
				if (ALLOCATE(cell->export_control_curve_dialog->file_name,char,
					strlen("cell.iptime")+1))
				{
					strcpy(cell->export_control_curve_dialog->file_name,"cell.iptime");
				}
        /* make the dialog shell */
        if (cell->export_control_curve_dialog->shell =
          XtVaCreatePopupShell("export_control_curves_dialog_shell",
            xmDialogShellWidgetClass,cell->shell,
            XmNmwmDecorations,MWM_DECOR_ALL,
            XmNmwmFunctions,MWM_FUNC_ALL,
            XmNdeleteResponse,XmDESTROY,
            XmNtransient,FALSE,
            NULL))
        {
          /* identify the shell for the busy icon */
          create_Shell_list_item(&(cell->export_control_curve_dialog->shell),
            cell->user_interface);
          /* add the destroy callback */
          XtAddCallback(cell->export_control_curve_dialog->shell,
						XmNdestroyCallback,export_control_curves_dialog_destroy_callback,
						(XtPointer)cell);
          WM_DELETE_WINDOW=XmInternAtom(
            XtDisplay(cell->export_control_curve_dialog->shell),
            "WM_DELETE_WINDOW",FALSE);
          XmAddWMProtocolCallback(cell->export_control_curve_dialog->shell,
            WM_DELETE_WINDOW,export_control_curves_dialog_destroy_callback,
						(XtPointer)cell);
          /* register cellbacks in UIL */
          if (MrmSUCCESS == MrmRegisterNamesInHierarchy(
            export_control_curves_dialog_hierarchy,callback_list,
						XtNumber(callback_list)))
          {
            /* set the identifier's values */
            width = cell->user_interface->screen_width/2;
            height = cell->user_interface->screen_height/4;
						identifier_list[0].value = (XtPointer)cell;
            identifier_list[1].value = (XtPointer)width;
            identifier_list[2].value = (XtPointer)height;
						identifier_list[3].value = (XtPointer)
							(XmStringCreateSimple(
								cell->export_control_curve_dialog->file_name));
            /* register the identifiers */
            if (MrmSUCCESS ==
              MrmRegisterNamesInHierarchy(
								export_control_curves_dialog_hierarchy,
                identifier_list,XtNumber(identifier_list)))
            {
              /* fetch the window widget */
              if (MrmSUCCESS == MrmFetchWidget(
								export_control_curves_dialog_hierarchy,
                "export_control_curves_dialog",
								cell->export_control_curve_dialog->shell,
                &(cell->export_control_curve_dialog->window),
								&export_control_curves_dialog_class))
              {
								if (create_control_curve_toggles(
									cell->export_control_curve_dialog,
									(cell->control_curve).control_curve_manager))
								{
									XtManageChild(cell->export_control_curve_dialog->window);
									XtRealizeWidget(cell->export_control_curve_dialog->shell);
									return_code = 1;
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"create_export_control_curves_dialog. "
										"Unable to create the variable widgets");
									DEALLOCATE(cell->export_control_curve_dialog);
									cell->export_control_curve_dialog = 
										(struct Export_control_curve_dialog *)NULL;
									return_code = 0;
								}
              }
              else
              {
                display_message(ERROR_MESSAGE,
									"create_export_control_curves_dialog. "
                  "Unable to fetch the window widget");
                DEALLOCATE(cell->export_control_curve_dialog);
                cell->export_control_curve_dialog = 
									(struct Export_control_curve_dialog *)NULL;
                return_code = 0;
              }
            }
            else
            {
              display_message(ERROR_MESSAGE,
								"create_export_control_curves_dialog. "
                "Unable to register the identifiers");
              DEALLOCATE(cell->export_control_curve_dialog);
              cell->export_control_curve_dialog = 
								(struct Export_control_curve_dialog *)NULL;
              return_code = 0;  
            }
          }
          else
          {
            display_message(ERROR_MESSAGE,
							"create_export_control_curves_dialog. "
              "Unable to register the callbacks");
            DEALLOCATE(cell->export_control_curve_dialog);
            cell->export_control_curve_dialog = 
							(struct Export_control_curve_dialog *)NULL;
            return_code = 0;  
          }
        }
        else
        {
					display_message(ERROR_MESSAGE,
						"create_export_control_curves_dialog. "
						"Unable to create the dialog shell for the export dialog");
					DEALLOCATE(cell->export_control_curve_dialog);
					cell->export_control_curve_dialog = 
						(struct Export_control_curve_dialog *)NULL;
					return_code = 0;          
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,
					"create_export_time_varaibles_dialog. "
          "Unable to allocate memory for the export dialog");
        cell->export_control_curve_dialog = 
					(struct Export_control_curve_dialog *)NULL;
        return_code = 0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"create_export_control_curves_dialog. "
        "Unable to open the export dialog hierarchy");
      cell->export_control_curve_dialog = 
				(struct Export_control_curve_dialog *)NULL;
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"create_export_control_curves_dialog. "
      "Missing Cell window");
    cell->export_control_curve_dialog = 
			(struct Export_control_curve_dialog *)NULL;
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END create_export_control_curves_dialog() */

/*
Global functions
================
*/

int bring_up_parameter_control_curve_dialog(struct Cell_window *cell,
  struct Cell_parameter *parameter)
/*******************************************************************************
LAST MODIFIED : 28 August 2000

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
					sprintf(name,"%s_xi1",parameter->spatial_label);
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
					sprintf(name,"%s_xi2",parameter->spatial_label);
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
					sprintf(name,"%s_xi3",parameter->spatial_label);
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

int bring_up_export_control_curves_dialog(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 29 September 1999

DESCRIPTION :
If a export window exists, pop it up, otherwise create it.
==============================================================================*/
{
  int return_code = 0;
  
  ENTER(bring_up_export_control_curves_dialog);
  if (cell != (struct Cell_window *)NULL)
  {
    if (cell->export_control_curve_dialog == 
			(struct Export_control_curve_dialog *)NULL)
    {
      /* Export window does not exist, so create it */
      if (create_export_control_curves_dialog(cell))
      {
        XtPopup(cell->export_control_curve_dialog->shell,XtGrabNone);
        return_code = 1;
      }
      else
      {
        display_message(ERROR_MESSAGE,"bring_up_export_control_curves_dialog. "
          "Error creating the export dialog");
        cell->export_control_curve_dialog = 
					(struct Export_control_curve_dialog *)NULL;
        return_code = 0;
      }
    }
    else
    {
      /* dialog already exists so just pop it up */
      XtPopup(cell->export_control_curve_dialog->shell,XtGrabNone);
      return_code = 1;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"bring_up_export_control_curves_dialog. "
      "Invalid arguments");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END bring_up_export_control_curves_dialog() */

void close_export_control_curves_dialog(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 29 September 1999

DESCRIPTION :
If there is a export dialog in existence, then destroy it.
==============================================================================*/
{
  ENTER(close_export_dialog);
  export_control_curves_dialog_destroy_callback((Widget)NULL,(XtPointer)cell,
    (XtPointer)NULL);
  LEAVE;
} /* END close_export_control_curves_dialog() */
