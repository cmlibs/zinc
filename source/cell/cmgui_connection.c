/*******************************************************************************
FILE : cmgui_connection.c

LAST MODIFIED : 17 September 1999

DESCRIPTION :
Functions for talking between Cell and Cmgui.
==============================================================================*/
#if defined (MOTIF)
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Xm/MwmUtil.h>
#include <Xm/DialogS.h>
#include <Xm/Protocols.h>
#endif /* if defined (MOTIF) */
#include "cell/cell_window.h"
#include "cell/cmgui_connection.h"
#include "cell/cell_parameter.h"
#include "cell/cell_variable.h"
#include "cell/parameter_dialog.h"
#include "cell/output.h"
#include "cell/input.h"
#include "cell/export_dialog.uid64"
#include "choose/choose_element_group.h"
#include "finite_element/import_finite_element.h"
#include "user_interface/filedir.h"

/*
Local types
===========
*/

/*
Module variables
================
*/
#if defined (MOTIF)
static int export_dialog_hierarchy_open=0;
static MrmHierarchy export_dialog_hierarchy;
#endif /* defined (MOTIF) */

/*
Local functions
===============
*/
static void identify_offset_textfield(Widget widget,
  XtPointer cell_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Identifies the text field widget for the offset value.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(identify_offset_textfield);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    cell->export_dialog->offset_textfield = widget;
  }
  else
  {
    display_message(ERROR_MESSAGE,"identify_offset_textfield. "
      "Missing cell window");
  }
  LEAVE;
} /* END identify_offset_textfield() */

static void identify_group_chooser_form(Widget widget,
  XtPointer cell_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 17 September 1999

DESCRIPTION :
Identifies the form for the group chooser widget to use.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(identify_group_chooser_form);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    cell->export_dialog->group_chooser_form = widget;
  }
  else
  {
    display_message(ERROR_MESSAGE,"identify_group_chooser_form. "
      "Missing cell window");
  }
  LEAVE;
} /* END identify_group_chooser_form() */

static void identify_ipcell_file_label(Widget widget,
  XtPointer cell_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Identifies the label widget for the ipcell file name
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(identify_ipcell_file_label);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    cell->export_dialog->ipcell_file_label = widget;
  }
  else
  {
    display_message(ERROR_MESSAGE,"identify_ipcell_file_label. "
      "Missing cell window");
  }
  LEAVE;
} /* END identify_ipcell_file_label() */

static void identify_ipmatc_file_label(Widget widget,
  XtPointer cell_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Identifies the label widget for the ipmatc file name
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(identify_ipmatc_file_label);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    cell->export_dialog->ipmatc_file_label = widget;
  }
  else
  {
    display_message(ERROR_MESSAGE,"identify_ipmatc_file_label. "
      "Missing cell window");
  }
  LEAVE;
} /* END identify_ipmatc_file_label() */

static void ipcell_browse_callback(Widget widget,
  XtPointer cell_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Callback for the browse button for the ipcell file.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  struct File_open_data *file_open_data;
  
  ENTER(ipcell_browse_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (file_open_data = create_File_open_data(".ipcell",REGULAR,
      file_open_ipcell_callback,(XtPointer)(cell->export_dialog),0,
      cell->user_interface))
    {
      open_file_and_write(widget,(XtPointer)file_open_data,call_data);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"ipcell_browse_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END ipcell_browse_callback() */

static void ipmatc_browse_callback(Widget widget,
  XtPointer cell_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Callback for the browse button for the ipmatc file.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  struct File_open_data *file_open_data;
  
  ENTER(ipmatc_browse_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (file_open_data = create_File_open_data(".ipmatc",REGULAR,
      file_open_ipmatc_callback,(XtPointer)(cell->export_dialog),0,
      cell->user_interface))
    {
      open_file_and_write(widget,(XtPointer)file_open_data,call_data);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"ipmatc_browse_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END ipmatc_browse_callback() */

static void ok_button_callback(Widget widget,
  XtPointer cell_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 17 September 1999

DESCRIPTION :
Callback for the OK button in the export dialog.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  struct FE_node *node = (struct FE_node *)NULL;
  struct GROUP(FE_element) *element_group;
  struct GROUP(FE_node) *node_group;
  char *name,*offset_value;
	int offset;
  
  ENTER(ok_button_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (cell->export_dialog && cell->export_dialog->shell)
    {
      XtPopdown(cell->export_dialog->shell);
			/* get the current offset value */
			if (cell->export_dialog->offset_textfield)
			{
				XtVaGetValues(cell->export_dialog->offset_textfield,
					XmNvalue,&offset_value,
					NULL);
				sscanf(offset_value,"%d",&offset);
			}
			else
			{
				offset = 0;
			}
      /* get the current node group - first get the name of the selected 
				 element group */
      if (element_group = CHOOSE_OBJECT_GET_OBJECT(GROUP(FE_element))(
				cell->export_dialog->group_chooser_widget))
      {
				if (GET_NAME(GROUP(FE_element))(element_group,&name))
				{
					/* now find the node group with the same name */
					if (node_group = FIND_BY_IDENTIFIER_IN_MANAGER(GROUP(FE_node),name)(
						name,(cell->cell_3d).node_group_manager))
					{
						/* now get the first node in the group to export to an ipcell 
							 file */
						if (node = FIRST_OBJECT_IN_GROUP_THAT(FE_node)(
							(GROUP_CONDITIONAL_FUNCTION(FE_node) *)NULL,(void *)NULL,
							node_group))
						{
							if (export_FE_node_to_ipcell(
								cell->export_dialog->ipcell_file_name,node,cell))
							{
								if (export_FE_node_group_to_ipmatc(
									cell->export_dialog->ipmatc_file_name,node_group,cell,node,
									offset))
								{
									display_message(INFORMATION_MESSAGE,"Done.");
								}
								else
								{
									display_message(ERROR_MESSAGE,"ok_button_callback. "
										"Unable to export the node group to an ipmatc file");
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"ok_button_callback. "
                  "Unable to export the node to an ipcell file");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"ok_button_callback. "
                "Unable to get the first node in the group");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"ok_button_callback. "
              "Unable to get the node group");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"ok_button_callback. "
						"Unable to get the element group name");
				}
      }
      else
      {
				display_message(ERROR_MESSAGE,"ok_button_callback. "
          "Unable to get the element group");
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"ok_button_callback. "
        "Missing export dialog");
    }
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
LAST MODIFIED : 17 September 1999

DESCRIPTION :
Callback for the CANCEL button in the export dialog
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(cancel_button_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (cell->export_dialog && cell->export_dialog->shell)
    {
      XtPopdown(cell->export_dialog->shell);
    }
    else
    {
      display_message(ERROR_MESSAGE,"cancel_button_callback. "
        "Missing export dialog");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"cancel_button_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END cancel_button_callback() */

static void export_dialog_destroy_callback(Widget widget,
  XtPointer cell_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 17 September 1999

DESCRIPTION :
Callback for when the export dialog is destroy via the window manager menu ??
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(export_dialog_destroy_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (cell->export_dialog)
    {
      if (cell->export_dialog->shell)
      {
        /* remove the shell from the shell list */
        destroy_Shell_list_item_from_shell(&(cell->export_dialog->shell),
          cell->user_interface);
        /* make sure the dialog is no longer visible */
        XtPopdown(cell->export_dialog->shell);
        /* Unmanage the shell */
        XtUnmanageChild(cell->export_dialog->shell);
      }
      DEALLOCATE(cell->export_dialog);
      cell->export_dialog = (struct Export_dialog *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"export_dialog_destroy_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END export_dialog_destroy_callback() */

static int create_export_dialog(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 17 September 1999

DESCRIPTION :
Create a new export dialog.
==============================================================================*/
{
  int return_code = 0;
  Atom WM_DELETE_WINDOW;
  MrmType export_dialog_class;
  static MrmRegisterArg callback_list[] = {
    {"identify_group_chooser_form",(XtPointer)identify_group_chooser_form},
    {"identify_ipcell_file_label",(XtPointer)identify_ipcell_file_label},
    {"identify_ipmatc_file_label",(XtPointer)identify_ipmatc_file_label},
    {"ipcell_browse_callback",(XtPointer)ipcell_browse_callback},
    {"ipmatc_browse_callback",(XtPointer)ipmatc_browse_callback},
    {"ok_button_callback",(XtPointer)ok_button_callback},
    {"cancel_button_callback",(XtPointer)cancel_button_callback},
		{"verify_text_field_modification",
		 (XtPointer)verify_text_field_modification},
		{"identify_offset_textfield",(XtPointer)identify_offset_textfield}
  }; /* callback_list */
  static MrmRegisterArg identifier_list[] = {
    {"cell_window_structure",(XtPointer)NULL},
    {"window_width",(XtPointer)NULL},
    {"window_height",(XtPointer)NULL},
    {"default_ipcell_file_name",(XtPointer)NULL},
    {"default_ipmatc_file_name",(XtPointer)NULL},
		{"default_offset_value",(XtPointer)NULL},
  }; /* identifier_list */
  unsigned int width,height;
	char offset_value[6];
  
  ENTER(create_export_dialog);
  if (cell != (struct Cell_window *)NULL)
  {
    if (cell->export_dialog != (struct Export_dialog *)NULL)
    {
      /* destroy any existing variables_dialog */
      export_dialog_destroy_callback((Widget)NULL,(XtPointer)cell,
        (XtPointer)NULL);
    }
    if (MrmOpenHierarchy_base64_string(export_dialog_uid64,
      &export_dialog_hierarchy,&export_dialog_hierarchy_open))
    {
      if (ALLOCATE(cell->export_dialog,struct Export_dialog,1))
      {
        /* initialise the structure */
        cell->export_dialog->shell = (Widget)NULL;
        cell->export_dialog->window = (Widget)NULL;
        cell->export_dialog->group_chooser_form = (Widget)NULL;
        cell->export_dialog->group_chooser_widget = (Widget)NULL;
				cell->export_dialog->ipcell_file_label = (Widget)NULL;
				cell->export_dialog->ipcell_file_name = (char *)NULL;
				cell->export_dialog->ipmatc_file_label = (Widget)NULL;
				cell->export_dialog->ipmatc_file_name = (char *)NULL;
				cell->export_dialog->offset_textfield = (Widget)NULL;
				/* set the default file names */
				if (ALLOCATE(cell->export_dialog->ipcell_file_name,char,
					strlen("cell.ipcell")) && 
					ALLOCATE(cell->export_dialog->ipmatc_file_name,char,
						strlen("cell.ipmatc")))
				{
					sprintf(cell->export_dialog->ipcell_file_name,"cell.ipcell\0");
					sprintf(cell->export_dialog->ipmatc_file_name,"cell.ipmatc\0");
				}
        /* make the dialog shell */
        if (cell->export_dialog->shell =
          XtVaCreatePopupShell("export_dialog_shell",
            xmDialogShellWidgetClass,cell->shell,
            XmNmwmDecorations,MWM_DECOR_ALL,
            XmNmwmFunctions,MWM_FUNC_ALL,
            XmNdeleteResponse,XmDESTROY,
            XmNtransient,FALSE,
            NULL))
        {
          /* identify the shell for the busy icon */
          create_Shell_list_item(&(cell->export_dialog->shell),
            cell->user_interface);
          /* add the destroy callback */
          XtAddCallback(cell->export_dialog->shell,XmNdestroyCallback,
            export_dialog_destroy_callback,(XtPointer)cell);
          WM_DELETE_WINDOW=XmInternAtom(
            XtDisplay(cell->export_dialog->shell),
            "WM_DELETE_WINDOW",FALSE);
          XmAddWMProtocolCallback(cell->export_dialog->shell,
            WM_DELETE_WINDOW,export_dialog_destroy_callback,(XtPointer)cell);
          /* register cellbacks in UIL */
          if (MrmSUCCESS == MrmRegisterNamesInHierarchy(
            export_dialog_hierarchy,callback_list,XtNumber(callback_list)))
          {
            /* set the identifier's values */
            width = cell->user_interface->screen_width/2;
            height = cell->user_interface->screen_height/5;
						sprintf(offset_value,"10000\0");
            identifier_list[0].value = (XtPointer)cell;
            identifier_list[1].value = (XtPointer)width;
            identifier_list[2].value = (XtPointer)height;
						identifier_list[3].value = (XtPointer)
							(XmStringCreateSimple(cell->export_dialog->ipcell_file_name));
						identifier_list[4].value = (XtPointer)
							(XmStringCreateSimple(cell->export_dialog->ipmatc_file_name));
						identifier_list[5].value = (XtPointer)offset_value;
            /* register the identifiers */
            if (MrmSUCCESS ==
              MrmRegisterNamesInHierarchy(export_dialog_hierarchy,
                identifier_list,XtNumber(identifier_list)))
            {
              /* fetch the window widget */
              if (MrmSUCCESS == MrmFetchWidget(export_dialog_hierarchy,
                "export_dialog",cell->export_dialog->shell,
                &(cell->export_dialog->window),&export_dialog_class))
              {
                if (cell->export_dialog->group_chooser_widget =
                  CREATE_CHOOSE_OBJECT_WIDGET(GROUP(FE_element))(
                    cell->export_dialog->group_chooser_form,
                    (struct GROUP(FE_element) *)NULL,
                    (cell->cell_3d).element_group_manager,
                    (MANAGER_CONDITIONAL_FUNCTION(GROUP(FE_element)) *)NULL))
								{
                  XtManageChild(cell->export_dialog->window);
                  XtRealizeWidget(cell->export_dialog->shell);
                  return_code = 1;
                }
                else
                {
                  display_message(ERROR_MESSAGE,"create_export_dialog. "
                    "Unable to create the group chooser widget");
                  DEALLOCATE(cell->export_dialog);
                  cell->export_dialog = (struct Export_dialog *)NULL;
                  return_code = 0;
                }
              }
              else
              {
                display_message(ERROR_MESSAGE,"create_export_dialog. "
                  "Unable to fetch the window widget");
                DEALLOCATE(cell->export_dialog);
                cell->export_dialog = (struct Export_dialog *)NULL;
                return_code = 0;
              }
            }
            else
            {
              display_message(ERROR_MESSAGE,"create_export_dialog. "
                "Unable to register the identifiers");
              DEALLOCATE(cell->export_dialog);
              cell->export_dialog = (struct Export_dialog *)NULL;
              return_code = 0;  
            }
          }
          else
          {
            display_message(ERROR_MESSAGE,"create_export_dialog. "
              "Unable to register the callbacks");
            DEALLOCATE(cell->export_dialog);
            cell->export_dialog = (struct Export_dialog *)NULL;
            return_code = 0;  
          }
        }
        else
        {
					display_message(ERROR_MESSAGE,"create_export_dialog. "
						"Unable to create the dialog shell for the export dialog");
					DEALLOCATE(cell->export_dialog);
					cell->export_dialog = (struct Export_dialog *)NULL;
					return_code = 0;          
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"create_export_dialog. "
          "Unable to allocate memory for the export dialog");
        cell->export_dialog = (struct Export_dialog *)NULL;
        return_code = 0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"create_export_dialog. "
        "Unable to open the export dialog hierarchy");
      cell->export_dialog = (struct Export_dialog *)NULL;
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"create_export_dialog. "
      "Missing Cell window");
    cell->export_dialog = (struct Export_dialog *)NULL;
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END create_export_dialog() */


/*
Global functions
================
*/
int check_model_id(struct Cell_window *cell,struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 24 September 1999

DESCRIPTION :
Checks the model_id field of the given <node> and returns 1 if this matches
the current model ID of the <cell>. Also checks for a cell_type field defined
at the node, since the model ID is a constant field and hence defined at all
nodes not just the cellular grid points.
==============================================================================*/
{
	int return_code = 0;
	struct FE_field_component field_component;
	char *model_id;

	ENTER(check_model_id);
	if (cell && node)
	{
		if (field_component.field = FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)(
      "model_id",(cell->cell_3d).fe_field_manager))
    {
      field_component.number = 0;
      if (FE_field_is_defined_at_node(field_component.field,node) &&
				get_FE_nodal_string_value(node,field_component.field,
					field_component.number,/*version*/0,FE_NODAL_VALUE,&model_id))
      {
        if ((model_id != (char *)NULL) &&
          (cell->current_model_id != (char *)NULL))
        {
          if (!strcmp(model_id,cell->current_model_id))
          {
						/* add a check for the cell_type field, to ensure that the node has
							 cell type fields associated with it */
						if (field_component.field = 
							FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)("cell_type",
								(cell->cell_3d).fe_field_manager))
						{
							field_component.number = 0;
							if (FE_field_is_defined_at_node(field_component.field,node) &&
								get_FE_nodal_value_as_string(node,field_component.field,
									field_component.number,/*version*/0,FE_NODAL_VALUE,&model_id))
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
							return_code = 0;
						}
          }
          else
          {
            return_code = 0;
          }
          DEALLOCATE(model_id);
        }
        else
        {
          return_code = 0;
        }
      }
    }
    else
    {
      return_code = 0;
    }
	}
	else
	{
		return_code = 0;
	}
	LEAVE;
	return(return_code);
} /* END check_model_id() */

int file_open_ipcell_callback(char *filename,XtPointer export_dialog)
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Callback for the file selection dialog for the ipcell file in the export dialog.
==============================================================================*/
{
  int return_code = 0;
  struct Export_dialog *export = (struct Export_dialog *)NULL;

  ENTER(file_open_ipcell_callback);
  if (export = (struct Export_dialog *)export_dialog)
  {
    /* set the label widget */
    if (export->ipcell_file_label)
    {
      XtVaSetValues(export->ipcell_file_label,
        XmNlabelString,XmStringCreateSimple(filename),
				NULL);
    }
    /* and save the file name */
    if (export->ipcell_file_name)
    {
      DEALLOCATE(export->ipcell_file_name);
    }
    if (ALLOCATE(export->ipcell_file_name,char,strlen(filename)))
    {
      sprintf(export->ipcell_file_name,"%s\0",filename);
      return_code = 1;
    }
    else
    {
      display_message(ERROR_MESSAGE,"file_open_ipcell_callback. "
        "Unable to allocate memory for the file name");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"file_open_ipcell_callback. "
      "Missing export dialog");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END file_open_ipcell_callback() */

int file_open_ipmatc_callback(char *filename,XtPointer export_dialog)
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Callback for the file selection dialog for the ipmatc file in the export dialog.
==============================================================================*/
{
  int return_code = 0;
  struct Export_dialog *export = (struct Export_dialog *)NULL;

  ENTER(file_open_ipmatc_callback);
  if (export = (struct Export_dialog *)export_dialog)
  {
    /* set the label widget */
    if (export->ipmatc_file_label)
    {
      XtVaSetValues(export->ipmatc_file_label,
        XmNlabelString,XmStringCreateSimple(filename),
				NULL);
    }
    /* and save the file name */
    if (export->ipmatc_file_name)
    {
      DEALLOCATE(export->ipmatc_file_name);
    }
    if (ALLOCATE(export->ipmatc_file_name,char,strlen(filename)))
    {
      sprintf(export->ipmatc_file_name,"%s\0",filename);
      return_code = 1;
    }
    else
    {
      display_message(ERROR_MESSAGE,"file_open_ipmatc_callback. "
        "Unable to allocate memory for the file name");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"file_open_ipmatc_callback. "
      "Missing export dialog");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END file_open_ipmatc_callback() */

int FE_node_to_Cell_window(struct Cell_window *cell,struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 14 September 1999

DESCRIPTION :
Extracts values from a FE_node and sets up the cell window.
==============================================================================*/
{
  int return_code = 0;
  struct Cell_parameter *parameter = (struct Cell_parameter *)NULL;
  struct Cell_variable *variable = (struct Cell_variable *)NULL;
  struct FE_field_component field_component;
  enum FE_field_type field_type;
  FE_value value;
  
  ENTER(FE_node_to_Cell_window);
  if ((cell != (struct Cell_window *)NULL) &&
    (node != (struct FE_node *)NULL))
  {
    /* get the model name for the node, and if it is not the current model,
			 need define the model defaults (i.e. read in the default file for that
			 model) */
    
    /* To start with, just check the current model for a matching ID, and
			 if it isn't, prompt the user to read it in */
		/* also need to check for a cell_type defined at the node */
    if (check_model_id(cell,node))
    {
      /* now loop through all fields getting the values from the node and
				 setting the value in the cell window */
      /* first the variables */
      variable = cell->variables;
      while (variable != (struct Cell_variable *)NULL)
      {
        if (field_component.field =
          FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)(
            variable->spatial_label,(cell->cell_3d).fe_field_manager))
        {
          field_component.number = 0;
          if (get_FE_nodal_FE_value_value(node,&field_component,/*version*/0,
            FE_NODAL_VALUE,&value))
          {
            /* set the current variable's value */
            variable->value = (float)value;
            /* all variables are defined at nodes, but are not spatially
							 varying */
          }
          else
          {
            display_message(WARNING_MESSAGE,"FE_node_to_Cell_window. "
              "Unable to get a value for the field: %s",
              variable->spatial_label);
          }
        }
        else
        {
          /* do nothing */
        }
        variable = variable->next;
      } /* while (variable)  */
      /* update the dialog boxes */
      reset_variable_values(cell->variables);
      /* now do the parameters */
      parameter = cell->parameters;
      while (parameter != (struct Cell_parameter *)NULL)
      {
        if (field_component.field =
          FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)(
            parameter->spatial_label,(cell->cell_3d).fe_field_manager))
        {
          field_component.number = 0;
          if (get_FE_nodal_FE_value_value(node,&field_component,/*version*/0,
            FE_NODAL_VALUE,&value))
          {
            /* set the current variable's value */
            parameter->value = (float)value;
            /* if the field is defined at each node, then it is a spatially
							 varying parameter */
            field_type = get_FE_field_FE_field_type(field_component.field);
            switch (field_type)
            {
              case GENERAL_FE_FIELD:
              {
                parameter->spatial_switch = 1;
								parameter->time_variable_allowed = 1;
              } break;
              case CONSTANT_FE_FIELD:
              case INDEXED_FE_FIELD:
              default:
              {
                parameter->spatial_switch = 0;
								parameter->time_variable_allowed = 0;
              } break;
            } /* switch (field_type) */
          }
          else
          {
            display_message(WARNING_MESSAGE,"FE_node_to_Cell_window. "
              "Unable to get a value for the field: %s",
              parameter->spatial_label);
          }
        }
        else
        {
          /* do nothing */
        }
        parameter = parameter->next;
      } /* while (parameter)  */
      /* update the dialog boxes */
      update_parameter_dialog_boxes(cell);
      return_code = 1;
    }
    else
    {
      display_message(INFORMATION_MESSAGE,"FE_node_to_Cell_window. "
        "Need to change the current cell model!!\n");
      return_code = 1;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"FE_node_to_Cell_window. "
      "Invalid arguments");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END FE_node_to_Cell_window() */

int Cell_window_to_FE_node(struct Cell_window *cell,
  struct FE_node *node_to_be_modified)
/*******************************************************************************
LAST MODIFIED : 14 September 1999

DESCRIPTION :
Re-sets the <node> values from the Cell window.
==============================================================================*/
{
  int return_code = 0,number_of_values,index;
  struct FE_node *node = (struct FE_node *)NULL;
  struct FE_field *field = (struct FE_field *)NULL;
  struct Cell_parameter *parameter = (struct Cell_parameter *)NULL;
  struct Cell_variable *variable = (struct Cell_variable *)NULL;
  struct FE_field_component field_component,indexer_field_component;
  enum FE_field_type field_type;
  FE_value value;
  
  ENTER(Cell_window_to_FE_node);
  if (cell && node_to_be_modified)
  {
    /* store up all field changes and send them all through after all changes
			 have been made */
    MANAGER_BEGIN_CACHE(FE_field)((cell->cell_3d).fe_field_manager);
    /* create a copy of the node want to change*/
    if(node = CREATE(FE_node)(0,node_to_be_modified))
    {
      /* loop through all variables, updating the nodal fields */
      variable = cell->variables;
      while (variable != (struct Cell_variable *)NULL)
      {
        if (field_component.field =
          FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)(
            variable->spatial_label,(cell->cell_3d).fe_field_manager))
        {
          field_component.number = 0;
          field_type = get_FE_field_FE_field_type(field_component.field);
          switch (field_type)
          {
            case GENERAL_FE_FIELD:
            {
              /* values stored at nodes, so can set the value directly into
								 the node */
              value = (FE_value)(variable->value);
              if (!set_FE_nodal_FE_value_value(node,&field_component,0,
                FE_NODAL_VALUE,value))
              {
                display_message(WARNING_MESSAGE,"Cell_window_to_FE_node. "
                  "Unable to set the value for the general FE field: %s",
                  variable->spatial_label);
              }
            } break;
            case CONSTANT_FE_FIELD:
            {
              /* need to set the field value */
              if (field = CREATE(FE_field)())
              {
                /* Shouldn't have to do this here */
                set_FE_field_name(field,
                  get_FE_field_name(field_component.field));
                MANAGER_COPY_WITHOUT_IDENTIFIER(FE_field,name)(field,
                  field_component.field);
                /* ?? always assume single component fields ?? */
                value = (FE_value)(variable->value);
                if (!set_FE_field_FE_value_value(field,0,value))
                {
                  display_message(WARNING_MESSAGE,"Cell_window_to_FE_node. "
                    "Unable to set the value for the constant FE field: %s",
                    variable->spatial_label);
                }
                MANAGER_MODIFY_NOT_IDENTIFIER(FE_field,name)(
                  field_component.field,field,(cell->cell_3d).fe_field_manager);
                DESTROY(FE_field)(&field);
              }
              else
              {
                display_message(WARNING_MESSAGE,"Cell_window_to_FE_node. "
                  "Unable to create a copy of the field");
              }
            } break;
            case INDEXED_FE_FIELD:
            {
              /* need to set the field value, based on the indexing field */
              if (field = CREATE(FE_field)())
              {
                /* Shouldn't have to do this here */
                set_FE_field_name(field,
                  get_FE_field_name(field_component.field));
                MANAGER_COPY_WITHOUT_IDENTIFIER(FE_field,name)(field,
                  field_component.field);
                if (get_FE_field_type_indexed(field,
                  &(indexer_field_component.field),&number_of_values))
                {
                  /* get the index value */
                  indexer_field_component.number = 0;
                  if (get_FE_nodal_int_value(node,&indexer_field_component,0,
                    FE_NODAL_VALUE,&index))
                  {
                    /* ?? always assume single component fields ?? */
                    value = (FE_value)(variable->value);
                    if (!set_FE_field_FE_value_value(field,index-1,value))
                    {
                      display_message(WARNING_MESSAGE,"Cell_window_to_FE_node. "
                        "Unable to set the value for the constant FE field: %s",
                        variable->spatial_label);
                    }
                    MANAGER_MODIFY_NOT_IDENTIFIER(FE_field,name)(
                      field_component.field,field,
                      (cell->cell_3d).fe_field_manager);
                  }
                  else
                  {
                    display_message(WARNING_MESSAGE,"Cell_window_to_FE_node. "
                      "Unable to get the index for the indexed FE field: "
                      "%s",variable->spatial_label);
                  }
                }
                else
                {
                  display_message(WARNING_MESSAGE,"Cell_window_to_FE_node. "
                    "Unable to get the indexer field for the indexed FE field: "
                    "%s",variable->spatial_label);
                }
                DESTROY(FE_field)(&field);
              }
              else
              {
                display_message(WARNING_MESSAGE,"Cell_window_to_FE_node. "
                  "Unable to create a copy of the field");
              }
            } break;
            default:
            {
            } break;
          } /* switch (field_type) */
        }
        else
        {
          /* do nothing */
        }
        variable = variable->next;
      } /* while (variable != (struct Cell_variable *)NULL) */
      /* loop through all parameters, updating the nodal fields */
      parameter = cell->parameters;
      while (parameter != (struct Cell_parameter *)NULL)
      {
        if (field_component.field =
          FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)(
            parameter->spatial_label,(cell->cell_3d).fe_field_manager))
        {
          field_component.number = 0;
          field_type = get_FE_field_FE_field_type(field_component.field);
          switch (field_type)
          {
            case GENERAL_FE_FIELD:
            {
              /* values stored at nodes, so can set the value directly into
								 the node */
              value = (FE_value)(parameter->value);
              if (!set_FE_nodal_FE_value_value(node,&field_component,0,
                FE_NODAL_VALUE,value))
              {
                display_message(WARNING_MESSAGE,"Cell_window_to_FE_node. "
                  "Unable to set the value for the general FE field: %s",
                  parameter->spatial_label);
              }
            } break;
            case CONSTANT_FE_FIELD:
            {
              /* need to set the field value */
              if (field = CREATE(FE_field)())
              {
                /* Shouldn't have to do this here */
                set_FE_field_name(field,
                  get_FE_field_name(field_component.field));
                MANAGER_COPY_WITHOUT_IDENTIFIER(FE_field,name)(field,
                  field_component.field);
                /* ?? always assume single component fields ?? */
                value = (FE_value)(parameter->value);
                if (!set_FE_field_FE_value_value(field,0,value))
                {
                  display_message(WARNING_MESSAGE,"Cell_window_to_FE_node. "
                    "Unable to set the value for the constant FE field: %s",
                    parameter->spatial_label);
                }
                MANAGER_MODIFY_NOT_IDENTIFIER(FE_field,name)(
                  field_component.field,field,(cell->cell_3d).fe_field_manager);
                DESTROY(FE_field)(&field);
              }
              else
              {
                display_message(WARNING_MESSAGE,"Cell_window_to_FE_node. "
                  "Unable to create a copy of the field");
              }
            } break;
            case INDEXED_FE_FIELD:
            {
              /* need to set the field value, based on the indexing field */
              if (field = CREATE(FE_field)())
              {
                /* Shouldn't have to do this here */
                set_FE_field_name(field,
                  get_FE_field_name(field_component.field));
                MANAGER_COPY_WITHOUT_IDENTIFIER(FE_field,name)(field,
                  field_component.field);
                if (get_FE_field_type_indexed(field,
                  &(indexer_field_component.field),&number_of_values))
                {
                  indexer_field_component.number = 0;
                  /* get the index value */
                  if (get_FE_nodal_int_value(node,&indexer_field_component,0,
                    FE_NODAL_VALUE,&index))
                  {
                    /* ?? always assume single component fields ?? */
                    value = (FE_value)(parameter->value);
                    if (!set_FE_field_FE_value_value(field,index-1,value))
                    {
                      display_message(WARNING_MESSAGE,"Cell_window_to_FE_node. "
                        "Unable to set the value for the constant FE field: %s",
                        parameter->spatial_label);
                    }
                    MANAGER_MODIFY_NOT_IDENTIFIER(FE_field,name)(
                      field_component.field,field,
                      (cell->cell_3d).fe_field_manager);
                  }
                  else
                  {
                    display_message(WARNING_MESSAGE,"Cell_window_to_FE_node. "
                      "Unable to get the index for the indexed FE field: "
                      "%s",parameter->spatial_label);
                  }
                }
                else
                {
                  display_message(WARNING_MESSAGE,"Cell_window_to_FE_node. "
                    "Unable to get the indexer field for the indexed FE field: "
                    "%s",parameter->spatial_label);
                }
                DESTROY(FE_field)(&field);
              }
              else
              {
                display_message(WARNING_MESSAGE,"Cell_window_to_FE_node. "
                  "Unable to create a copy of the field");
              }
            } break;
            default:
            {
            } break;
          } /* switch (field_type) */
        }
        else
        {
          /* do nothing */
        }
        parameter = parameter->next;
      } /* while (parameter != (struct Cell_parameter *)NULL) */
      /* modify the original node */
      MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)(
        node_to_be_modified,node,(cell->cell_3d).node_manager);
      DESTROY(FE_node)(&node);
      return_code = 1;
    }
    else
    {
      display_message(ERROR_MESSAGE,"Cell_window_to_FE_node. "
        "Unable to create a copy of the node");
      return_code = 0;
    }
    MANAGER_END_CACHE(FE_field)((cell->cell_3d).fe_field_manager);
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_window_to_FE_node. "
      "Invalid arguments");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END Cell_window_to_FE_node() */

int bring_up_export_window(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 17 September 1999

DESCRIPTION :
If a export window exists, pop it up, otherwise create it.
==============================================================================*/
{
  int return_code = 0;
  
  ENTER(bring_up_export_window);
  if (cell != (struct Cell_window *)NULL)
  {
    if (cell->export_dialog == (struct Export_dialog *)NULL)
    {
      /* Export window does not exist, so create it */
      if (create_export_dialog(cell))
      {
        XtPopup(cell->export_dialog->shell,XtGrabNone);
        return_code = 1;
      }
      else
      {
        display_message(ERROR_MESSAGE,"bring_up_export_window. "
          "Error creating the export dialog");
        cell->export_dialog = (struct Export_dialog *)NULL;
        return_code = 0;
      }
    }
    else
    {
      /* dialog already exists so just pop it up */
      XtPopup(cell->export_dialog->shell,XtGrabNone);
      return_code = 1;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"bring_up_export_window. "
      "Invalid arguments");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END bring_up_export_window() */

void close_export_dialog(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 17 September 1999

DESCRIPTION :
If there is a export dialog in existence, then destroy it.
==============================================================================*/
{
  ENTER(close_export_dialog);
  export_dialog_destroy_callback((Widget)NULL,(XtPointer)cell,
    (XtPointer)NULL);
  LEAVE;
} /* END close_export_dialog() */

int check_field_type(struct FE_field *field,void *field_type_void)
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Returns true if the FIELD_TYPE specified in the <field_type_void> matches
the FIELD_TYPE of the <field>.
==============================================================================*/
{
	int return_code = 0;
	enum FE_field_type field_type;

	ENTER(check_field_type);
	if (field && (field_type = (enum FE_field_type)field_type_void))
	{
		if (field_type == get_FE_field_FE_field_type(field))
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
		display_message(ERROR_MESSAGE,"check_field_type. "
			"Invalid arguments");
		return_code = 0;
	}
	LEAVE;
	return(return_code);
} /* END check_field_type() */
