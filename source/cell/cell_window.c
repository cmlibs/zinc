/*******************************************************************************
FILE : cell_window.c

LAST MODIFIED : 15 June 2000

DESCRIPTION :
Functions for using the Cell_window structure.
==============================================================================*/
#include <stdio.h>
#if defined (MOTIF)
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Xm/Command.h>
#include <Xm/DialogS.h>
#include <Xm/MwmUtil.h>
#include <Xm/List.h>
#include <Xm/Protocols.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#endif /* if defined (MOTIF) */
#include "cell/model_dialog.h"
#include "cell/calculate.h"
#include "cell/cell_window.h"
#include "cell/cell_window.uidh"
#include "cell/cell_component.h"
#include "cell/input.h"
#include "cell/parameter_dialog.h"
#include "cell/output.h"
#include "cell/cell_3d.h"
#include "cell/cell_variable.h"
#include "cell/cmgui_connection.h"
#include "choose/choose_enumerator.h"
#include "choose/text_choose_fe_element.h"
#include "choose/choose_computed_field.h"
#include "command/command.h"
#include "unemap/unemap_package.h"
#include "user_interface/user_interface.h"
#include "user_interface/filedir.h"
#include "graphics/scene_viewer.h"
#include "graphics/scene.h"
#include "graphics/colour.h"
#include "graphics/import_graphics_object.h"
#include "finite_element/import_finite_element.h"

/*
Module types
============
*/
typedef struct Cell_user_settings User_settings;

/*
Module variables
================
*/
#if defined (MOTIF)
static int cell_window_hierarchy_open=0;
static MrmHierarchy cell_window_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
================
*/
#if defined (OLD_CODE)
static void destroy_Cell_window(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 01 February 1999

DESCRIPTION :
Destroy the cell_window structure and remove the window
==============================================================================*/
{
  struct Cell_window *cell;
  
  ENTER(destroy_Cell_window);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (cell->output_file)
    {
      fclose(cell->output_file);
    }
    destroy_Shell_list_item_from_shell(&(cell->shell),
      cell->user_interface);
    XtPopdown(cell->shell);
    XtUnmanageChild(cell->shell);
    DEALLOCATE(cell);
  }
  else
  {
    display_message(ERROR_MESSAGE,"destroy_Cell_window. "
      "Missing cell_window structure");
  }
  LEAVE;
} /* END destroy_Cell_window() */
#endif /* defined (OLD_CODE) */

static void cell_window_close(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 16 Septemeber 1999

DESCRIPTION :
Called when the Close function is selected from the window manager menu in the
Cell window.
==============================================================================*/
{
  struct Cell_window *cell;
  struct Cell_component *component;

  ENTER(cell_window_close);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    XtPopdown(cell->shell);
    /* close all dialogs */
    close_model_dialog(cell);
    close_variables_dialog(cell);
    close_export_dialog(cell);
    close_export_control_curves_dialog(cell);
    component = cell->components;
    while (component != (struct Cell_component *)NULL)
    {
      close_parameter_dialog(component);
      component = component->next;
    }
#if defined (OLD_CODE)
    close_user_interface(cell->user_interface);
    if (cell->output_file)
    {
      fclose(cell->output_file);
    }
    DEALLOCATE(cell);
    exit(0);
#endif /* defined (OLD_CODE) */
  }
  else
  {
    display_message(ERROR_MESSAGE,"cell_window_close. "
      "Missing cell window");
  }
  LEAVE;
} /* END cell_window_close() */

static void identify_output_pane(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 02 February 1999

DESCRIPTION :
Stores the id of the message areas.
==============================================================================*/
{
  struct Cell_window *cell;
  
  ENTER(identify_output_pane);
	USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    cell->output_pane = widget;
  }
  else
  {
    display_message(ERROR_MESSAGE,"identify_output_pane. "
      "Missing Cell window");
  }
  LEAVE;
} /* END identify_output_pane() */

#if defined (CELL_USE_NODES)
static void identify_node_chooser_label(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 September 1999

DESCRIPTION :
Stores the id of the node chooser label.
==============================================================================*/
{
  struct Cell_window *cell;
  
  ENTER(identify_node_chooser_label);
	USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    (cell->distributed).node_chooser_label = widget;
  }
  else
  {
    display_message(ERROR_MESSAGE,"identify_node_chooser_label. "
      "Missing Cell window");
  }
  LEAVE;
} /* END identify_node_chooser_label() */
#endif /* defined (CELL_USE_NODES) */

static void identify_element_form(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 08 June 2000

DESCRIPTION :
Stores the id of the element chooser form.
==============================================================================*/
{
  struct Cell_window *cell;
  
  ENTER(identify_element_form);
	USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    (cell->distributed).element_form = widget;
  }
  else
  {
    display_message(ERROR_MESSAGE,"identify_element_form. "
      "Missing Cell window");
  }
  LEAVE;
} /* END identify_element_form() */

static void identify_point_number_text(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 08 June 2000

DESCRIPTION :
Stores the id of the element point number text field.
==============================================================================*/
{
  struct Cell_window *cell;
  
  ENTER(identify_point_number_text);
	USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    (cell->distributed).point_number_text = widget;
  }
  else
  {
    display_message(ERROR_MESSAGE,"identify_point_number_text. "
      "Missing Cell window");
  }
  LEAVE;
} /* END identify_point_number_text() */

static void identify_grid_field_form(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 08 June 2000

DESCRIPTION :
Stores the id of the grid field chooser form.
==============================================================================*/
{
  struct Cell_window *cell;
  
  ENTER(identify_grid_field_form);
	USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    (cell->distributed).grid_field_form = widget;
  }
  else
  {
    display_message(ERROR_MESSAGE,"identify_grid_field_form. "
      "Missing Cell window");
  }
  LEAVE;
} /* END identify_grid_field_form() */

static void identify_grid_value_text(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 08 June 2000

DESCRIPTION :
Stores the id of the grid field value text field.
==============================================================================*/
{
  struct Cell_window *cell;
  
  ENTER(identify_grid_value_text);
	USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    (cell->distributed).grid_value_text = widget;
  }
  else
  {
    display_message(ERROR_MESSAGE,"identify_grid_value_text. "
      "Missing Cell window");
  }
  LEAVE;
} /* END identify_grid_value_text() */

static void identify_export_button(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Stores the id of the export button in the file -> write menu.
==============================================================================*/
{
  struct Cell_window *cell;
  
  ENTER(identify_export_button);
	USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    (cell->distributed).export_menu_button = widget;
  }
  else
  {
    display_message(ERROR_MESSAGE,"identify_export_button. "
      "Missing Cell window");
  }
  LEAVE;
} /* END identify_export_button() */

#if defined (CELL_USE_NODES)
static void identify_node_chooser_form(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 September 1999

DESCRIPTION :
Stores the id of the node chooser.
==============================================================================*/
{
  struct Cell_window *cell;
  
  ENTER(identify_node_chooser_form);
	USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    (cell->distributed).node_chooser_form = widget;
  }
  else
  {
    display_message(ERROR_MESSAGE,"identify_node_chooser_form. "
      "Missing Cell window");
  }
  LEAVE;
} /* END identify_node_chooser_form() */
#endif /* defined (CELL_USE_NODES) */

static void identify_description_label(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 September 1999

DESCRIPTION :
Stores the id of the description label for the distributed modelling.
==============================================================================*/
{
  struct Cell_window *cell;
  
  ENTER(identify_description_label);
	USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    (cell->distributed).description_label = widget;
  }
  else
  {
    display_message(ERROR_MESSAGE,"identify_description_label. "
      "Missing Cell window");
  }
  LEAVE;
} /* END identify_description_label() */

static void identify_apply_button(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 September 1999

DESCRIPTION :
Stores the id of the apply button for the distributed modelling.
==============================================================================*/
{
  struct Cell_window *cell;
  
  ENTER(identify_apply_button);
	USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    (cell->distributed).apply_button = widget;
  }
  else
  {
    display_message(ERROR_MESSAGE,"identify_apply_button. "
      "Missing Cell window");
  }
  LEAVE;
} /* END identify_apply_button() */

static void identify_reset_button(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 September 1999

DESCRIPTION :
Stores the id of the reset button for the distributed modelling.
==============================================================================*/
{
  struct Cell_window *cell;
  
  ENTER(identify_reset_button);
	USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    (cell->distributed).reset_button = widget;
  }
  else
  {
    display_message(ERROR_MESSAGE,"identify_reset_button. "
      "Missing Cell window");
  }
  LEAVE;
} /* END identify_reset_button() */

static void identify_drawing_area(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 16 February 1999

DESCRIPTION :
Stores the id of the drawing area.
==============================================================================*/
{
  struct Cell_window *cell;
  
  ENTER(identify_drawing_area);
	USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    (cell->image_map).drawing_area = widget;
  }
  else
  {
    display_message(ERROR_MESSAGE,"identify_drawing_area. "
      "Missing Cell window");
  }
  LEAVE;
} /* END identify_drawing_area() */

static void identify_cell_3d_form(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 04 September 1999

DESCRIPTION :
Stores the id of the form which will contain the Cell 3D scene viewer.
==============================================================================*/
{
  struct Cell_window *cell;
  
  ENTER(identify_cell_3d_form);
	USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    (cell->cell_3d).form = widget;
  }
  else
  {
    display_message(ERROR_MESSAGE,"identify_cell_3d_form. "
      "Missing Cell window");
  }
  LEAVE;
} /* END identify_cell_3d_form() */

static void save_toggle_changed_callback(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 February 1999

DESCRIPTION :
Callback for when the state of the save toggle changes.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  Boolean state;

  ENTER(save_toggle_changed_callback);
	USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    XtVaGetValues(widget,
      XmNset,&state,
      NULL);
    if (state)
    {
      (cell->menu).save = 1;
    }
    else
    {
      (cell->menu).save = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"save_toggle_changed_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END save_toggle_changed_callback() */

#if defined (CELL_USE_NODES)
static void cell_window_update_node(Widget widget,
	void *cell_window,void *node_void)
/*******************************************************************************
LAST MODIFIED : 14 September 1999

DESCRIPTION :
Callback for change of node from text_choose_fe_node.
==============================================================================*/
{
  char *name,description[100];
  int cell_type;
	struct FE_node *node;
  struct FE_field_component field_component;
  struct Cell_window *cell = (struct Cell_window *)NULL;
  XmString str;

	ENTER(cell_window_update_node);
	USE_PARAMETER(widget);
  if (cell = (struct Cell_window *)cell_window)
  {
    node=(struct FE_node *)node_void;
    if (GET_NAME(FE_node)(node,&name))
    {
      if (field_component.field =
        FIND_BY_IDENTIFIER_IN_MANAGER(FE_field,name)("cell_type",
          (cell->cell_3d).fe_field_manager))
      {
        field_component.number = 0;
        if (FE_field_is_defined_at_node(field_component.field,node) &&
					get_FE_nodal_int_value(node,&field_component,0,FE_NODAL_VALUE,
          &cell_type))
				{
					sprintf(description,"Currently editing node %s, which has a "
						"cell type "
						"%d\0",name,cell_type);
					DEALLOCATE(name);
				}
				else
				{
					sprintf(description,"Invalid node");
				}
				str = XmStringCreateSimple(description);
        if ((cell->distributed).description_label != (Widget)NULL)
        {
          XtVaSetValues((cell->distributed).description_label,
            XmNlabelString,str,
            NULL);
        }
        XmStringFree(str);
      }
    }
    /* put node into cell window */
    if (!FE_node_to_Cell_window(cell,node))
    {
      display_message(ERROR_MESSAGE,"cell_window_update_node. "
        "Unable to update the cell window from the node");
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cell_window_update_node.  Invalid argument(s)");
	}
	LEAVE;
} /* cell_window_update_node */
#endif /* defined (CELL_USE_NODES) */

static void Cell_window_update_from_element_point(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 14 June 2000

DESCRIPTION :
Called whenever the cell window needs to be updated from the element point
currently selected.
==============================================================================*/
{
  char *value_string,description[100];
  XmString str;
  struct Computed_field *field;
  FE_value *xi;
  int comp_no;
  struct FE_element *element,*top_level_element;

	ENTER(Cell_window_update_from_element_point);
  if (cell &&
    (element=(cell->distributed).element_point_identifier.element)&&
		(top_level_element=
			(cell->distributed).element_point_identifier.top_level_element)&&
		(xi=(cell->distributed).xi))
  {
    comp_no = 0;
    /* get the cell_type field if possible */
    if (field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)("cell_type",
      Computed_field_package_get_computed_field_manager(
				cell->computed_field_package)))
    {
      if (value_string = Computed_field_evaluate_component_as_string_in_element(
        field,comp_no,element,xi,top_level_element))
      {
        sprintf(description,"Currently editing a grid point with cell type: "
          "%s\0",value_string);
        DEALLOCATE(value_string);
      }
    }
    else
    {
      sprintf(description,"Can't get the cell type for this grid point");
    }
    str = XmStringCreateSimple(description);
    if ((cell->distributed).description_label != (Widget)NULL)
    {
      XtVaSetValues((cell->distributed).description_label,
        XmNlabelString,str,
        NULL);
    }
    XmStringFree(str);
    /* put element point into cell window */
    if (!element_point_to_Cell_window(cell->computed_field_package,comp_no,
      element,xi,top_level_element,cell))
    {
      display_message(ERROR_MESSAGE,"Cell_window_update_from_element_point. "
        "Unable to update the cell window from the element point");
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cell_window_update_from_element_point.  Invalid argument(s)");
	}
	LEAVE;
} /* Cell_window_update_from_element_point */

static void edit_dist_changed_callback(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 September 1999

DESCRIPTION :
Callback for when the state of the edit distributed toggle changes
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
#if defined (CELL_USE_NODES)
  struct FE_node *node = (struct FE_node *)NULL;
#endif /* defined (CELL_USE_NODES) */
  Boolean state;

  ENTER(edit_dist_changed_callback);
	USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    XtVaGetValues(widget,
      XmNset,&state,
      NULL);
    if (state)
    {
      (cell->distributed).edit = 1;
      /* update from the current selected element point */
      Cell_window_update_from_element_point(cell);
    }
    else
    {
      (cell->distributed).edit = 0;      
    }
    XtSetSensitive((cell->distributed).description_label,state);
    XtSetSensitive((cell->distributed).element_widget,state);
    XtSetSensitive((cell->distributed).point_number_text,state);
    XtSetSensitive((cell->distributed).grid_field_widget,state);
    XtSetSensitive((cell->distributed).grid_value_text,state);
    XtSetSensitive((cell->distributed).apply_button,state);
    XtSetSensitive((cell->distributed).reset_button,state);
		XtSetSensitive((cell->distributed).export_menu_button,state);
#if defined (CELL_USE_NODES)
    XtSetSensitive((cell->distributed).node_chooser_label,state);
    XtSetSensitive((cell->distributed).node_chooser_widget,state);
#endif /* defined (CELL_USE_NODES) */
  }
  else
  {
    display_message(ERROR_MESSAGE,"edit_dist_changed_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END edit_dist_changed_callback() */

#if defined (CELL_USE_NODES)
static void cell_node_group_change(
  struct MANAGER_MESSAGE(GROUP(FE_node)) *message,void *cell_window)
/*******************************************************************************
LAST MODIFIED : 14 September 1999

DESCRIPTION :
Node group manager change callback. Adds the first node put into the active
group to the node chooser in Cell.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  struct FE_node *node;
  
  ENTER(cell_node_group_change);
  if (cell = (struct Cell_window *)cell_window)
  {
    if ((cell->distributed).edit)
    {
      /* only care about changes if user wants to edit distributed model
      parameters */
      switch (message->change)
      {
        case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(GROUP(FE_node)):
        {
          /* Add the node to the cell node chooser and update cell */
          if (node = FIRST_OBJECT_IN_GROUP_THAT(FE_node)(
            (GROUP_CONDITIONAL_FUNCTION(FE_node) *)NULL,(void *)NULL,
            message->object_changed))
          {
            TEXT_CHOOSE_OBJECT_SET_OBJECT(FE_node)(
              (cell->distributed).node_chooser_widget,node);
            cell_window_update_node((Widget)NULL,(void *)cell,(void *)node);
          }
        } break;
        case MANAGER_CHANGE_ALL(GROUP(FE_node)):
        case MANAGER_CHANGE_OBJECT(GROUP(FE_node)):
        case MANAGER_CHANGE_DELETE(GROUP(FE_node)):
        case MANAGER_CHANGE_ADD(GROUP(FE_node)):
        case MANAGER_CHANGE_IDENTIFIER(GROUP(FE_node)):
        default:
        {
          /* ?? do nothing ?? */
        } break;
      }
    } /* if (edit) */
  }
  else
  {
    display_message(ERROR_MESSAGE,"cell_node_group_change. "
      "Missing cell window");
  }
  LEAVE;
} /* END cell_node_group_change() */
#endif /* defined (CELL_USE_NODES) */

static void input_mode_callback(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 06 September 1999

DESCRIPTION :
Callback for when the state of the input mode button is activated - used to
set the scene viewer input mode for Cell 3D.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  enum Scene_viewer_input_mode input_mode;
  XmString str;
  
  ENTER(input_mode_callback);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    input_mode = Scene_viewer_get_input_mode((cell->cell_3d).scene_viewer);
    switch (input_mode)
    {
      case SCENE_VIEWER_SELECT:
      {
        if (!Scene_viewer_set_input_mode((cell->cell_3d).scene_viewer,
          SCENE_VIEWER_TRANSFORM))
        {
          display_message(ERROR_MESSAGE,"input_mode_callback. "
            "Unable to set the input mode");
        }
        else
        {
          str = XmStringCreateSimple("Transform");
          XtVaSetValues(widget,
            XmNlabelString,str,
            NULL);
        }
      } break;
      case SCENE_VIEWER_TRANSFORM:
      {
        if (!Scene_viewer_set_input_mode((cell->cell_3d).scene_viewer,
          SCENE_VIEWER_SELECT))
        {
          display_message(ERROR_MESSAGE,"input_mode_callback. "
            "Unable to set the input mode");
        }
        else
        {
          str = XmStringCreateSimple("Select");
          XtVaSetValues(widget,
            XmNlabelString,str,
            NULL);
        }
      } break;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"input_mode_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END input_mode_callback() */

static void debug_toggle_changed_callback(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 28 February 1999

DESCRIPTION :
Callback for when the state of the save toggle changes.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  Boolean state;

  ENTER(debug_toggle_changed_callback);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    XtVaGetValues(widget,
      XmNset,&state,
      NULL);
    if (state)
    {
      (cell->menu).debug = 1;
    }
    else
    {
      (cell->menu).debug = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"debug_toggle_changed_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END debug_toggle_changed_callback() */

static void open_model_callback(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 27 February 1999

DESCRIPTION :
callback for the open model button in the file menu.
==============================================================================*/
{
  struct Cell_window *cell;
  
  ENTER(open_model_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    if(!bring_up_model_dialog(cell))
    {
      display_message(ERROR_MESSAGE,"open_model_callback. "
        "Unable to bring up the model dialog");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"open_model_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END open_model_callback() */

#if 0
static void file_write_cmiss_callback(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 03 February 1999

DESCRIPTION :
Callback for the "Write->CMISS" choice in the "File" menu.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(file_write_cmiss_callback);
  if (cell = (struct Cell_window *)cell_window)
  {
    display_message(INFORMATION_MESSAGE,"file_write_cmiss_callback.\n");
  }
  else
  {
    display_message(ERROR_MESSAGE,"file_write_cmiss_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END file_write_cmiss_callback() */
#endif

static void calculate_button_callback(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Callback for the "Calculate" button in the menu bar.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
	char *define_commands[] = {
		"fem define parameters;r;cell_tmp",
		"fem define bases;d",
		"fem define nodes;d",
		"fem define elements;d",
		"fem define grid;d",
		"fem update grid geometry",
		"fem update grid metric",
		"fem define equation;r;cell_tmp",
		"fem define cell;r;cell_tmp",
		"fem define material;r;cell_tmp cell",
		"fem define material;d",
		"fem define initial;r;cell_tmp",
		"fem define solve;r;cell_tmp"
	};
	int number_of_define_commands = 13;
	char *initialise_commands[] = {
		"fem solve to 0"
	};
	int number_of_initialise_commands = 1;
	char *finish_commands[] = {
		"fem close history binary",
		"fem evaluate electrode;cell_tmp history cell_tmp from grid yq binary",
		"fem define export;r;cell_tmp",
		"fem export signal;cell_tmp electrode signal cell_tmp"
	};
	int number_of_finish_commands = 4;
	char command[250];
	int command_number,return_code;
	int time;
  
  ENTER(calculate_button_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
		/* turn on the busy cursor */
		busy_cursor_on((Widget)NULL,cell->user_interface);

		/* create the files required by CMISS */
		write_ippara_file("cell_tmp.ippara");
		write_ipequa_file("cell_tmp.ipequa",cell->current_model);
		write_cmiss_file("cell_tmp.ipcell",(XtPointer)cell);
		write_ipmatc_file("cell_tmp.ipmatc");
		write_ipinit_file("cell_tmp.ipinit");
		write_ipsolv_file("cell_tmp.ipsolv");
		write_ipexpo_file("cell_tmp.ipexpo");

		/* execute the commands required to define the model */
		command_number = 0;
		return_code = 1;
		for (command_number=0;return_code && 
					 (command_number<number_of_define_commands);command_number++)
		{
			return_code = Execute_command_execute_string(cell->execute_command,
				define_commands[command_number]);
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,"calculate_button_callback. "
					"There was an error defining the model");	
				display_message(INFORMATION_MESSAGE,"Bad command: %s\n",
					define_commands[command_number]);
			}
		}
		/* execute the commands required to intialise the model */
		command_number = 0;
		for (command_number=0;return_code && 
					 (command_number<number_of_initialise_commands);command_number++)
		{
			return_code = Execute_command_execute_string(cell->execute_command,
				initialise_commands[command_number]);
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,"calculate_button_callback. "
					"There was an error initialising the model");	
				display_message(INFORMATION_MESSAGE,"Bad command: %s\n",
					initialise_commands[command_number]);
			}
		}
		/* open the history file - for now only use the potential, but eventually
			 want to write out all variables ?? */
		sprintf(command,"fem open history;cell_tmp write variables yq niqlist 1 "
			"binary\0");
		return_code = Execute_command_execute_string(cell->execute_command,
			command);
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,"calculate_button_callback. "
				"There was an error opening the history file");	
			display_message(INFORMATION_MESSAGE,"Bad command: %s\n",command);
		}
		/* now solve the model - eventually want to get the start/stop/steps etc..
			 from the user ?? */
		for (time=0;return_code && (time<300);time++)
		{
			sprintf(command,"fem solve restart to %d",time);
			return_code = Execute_command_execute_string(cell->execute_command,
				command);
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,"calculate_button_callback. "
					"There was an error solving the model");	
				display_message(INFORMATION_MESSAGE,"Bad command: %s\n",command);
			}
			else
			{
				sprintf(command,"fem write history time %d variables yq binary",time);
				return_code = Execute_command_execute_string(cell->execute_command,
					command);
				if (!return_code)
				{
					display_message(ERROR_MESSAGE,"calculate_button_callback. "
						"There was an error writing to the history file");
					display_message(INFORMATION_MESSAGE,"Bad command: %s\n",command);
				}
			}
		} /* time loop */
		/* execute the commands required to finish the model */
		command_number = 0;
		for (command_number=0;return_code && 
					 (command_number<number_of_finish_commands);command_number++)
		{
			return_code = Execute_command_execute_string(cell->execute_command,
				finish_commands[command_number]);
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,"calculate_button_callback. "
					"There was an error finishing the model");	
				display_message(INFORMATION_MESSAGE,"Bad command: %s\n",
					finish_commands[command_number]);
			}
		}

		/* turn off the busy cursor */
		busy_cursor_off((Widget)NULL,cell->user_interface);
		
  }
  else
  {
    display_message(ERROR_MESSAGE,"calculate_button_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END calculate_button_callback() */

static void clear_button_callback(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 12 February 1999

DESCRIPTION :
Callback for the "Clear" button in the menu bar.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(clear_button_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    clear_signals_window((Widget *)NULL,(XtPointer)cell,(XtPointer)NULL);
  }
  else
  {
    display_message(ERROR_MESSAGE,"clear_button_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END clear_button_callback() */

static void model_button_callback(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 03 February 1999

DESCRIPTION :
Callback for the "Model" button in the menu bar.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(model_button_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (!bring_up_model_dialog(cell))
    {
      display_message(ERROR_MESSAGE,"model_button_callback. "
        "Unable to bring up model dialog");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"model_button_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END model_button_callback() */

static void variables_button_callback(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 05 February 1999

DESCRIPTION :
Callback for the "Variables" button in the menu bar.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(variables_button_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (!bring_up_variables_dialog(cell))
    {
      display_message(ERROR_MESSAGE,"variables_button_callback. "
        "Unable to bring up variables dialog");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"variables_button_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END variables_button_callback() */

static void currents_button_callback(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 03 February 1999

DESCRIPTION :
Callback for the "Currents" button in the menu bar.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(currents_button_callback);
  USE_PARAMETER(cell);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    display_message(INFORMATION_MESSAGE,"currents_button_callback.\n");
  }
  else
  {
    display_message(ERROR_MESSAGE,"currents_button_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END currents_button_callback() */

static void parameters_button_callback(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 09 March 1999

DESCRIPTION :
Callback for the "Parameters" button in the menu bar.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  struct Cell_component *current = (struct Cell_component *)NULL;
  int found = 0;
  
  ENTER(parameters_button_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    current = cell->components;
    while ((current != (struct Cell_component *)NULL) && !found)
    {
      /*bring_up_parameter_dialog(current);*/
      if (!strcmp(current->component,"Parameters"))
      {
        found = 1;
      }
      else
      {
        current = current->next;
      }
    }
    if (found && (current != (struct Cell_component *)NULL))
    {
      bring_up_parameter_dialog(current);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"parameters_button_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END parameters_button_callback() */

static void help_index_callback(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 03 February 1999

DESCRIPTION :
Callback for the "Index" button in the "Help" menu.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(help_index_callback);
  USE_PARAMETER(cell);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    display_message(INFORMATION_MESSAGE,"help_index_callback.\n");
  }
  else
  {
    display_message(ERROR_MESSAGE,"help_index_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END help_index_callback() */

static void help_describe_parameter_callback(Widget widget,
  XtPointer cell_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 03 February 1999

DESCRIPTION :
Callback for the "Describe->parameter" button in the "Help" menu.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(help_describe_parameter_callback);
  USE_PARAMETER(cell);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    display_message(INFORMATION_MESSAGE,"help_describe_parameter_callback.\n");
  }
  else
  {
    display_message(ERROR_MESSAGE,"help_describe_parameter_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END help_describe_parameter_callback() */

static void help_describe_component_callback(Widget widget,
  XtPointer cell_window,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 03 February 1999

DESCRIPTION :
Callback for the "Describe->component" button in the "Help" menu.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(help_describe_component_callback);
  USE_PARAMETER(cell);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    display_message(INFORMATION_MESSAGE,"help_describe_component_callback.\n");
  }
  else
  {
    display_message(ERROR_MESSAGE,"help_describe_component_callback. "
      "Missing Cell window");
  }
  LEAVE;
} /* END help_describe_component_callback() */

#if defined (OLD_CODE)
static void drawing_area_input(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 17 February 1999

DESCRIPTION :
Callback for input events in the drawing area.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  struct Cell_component *current = (struct Cell_component *)NULL;
  XmDrawingAreaCallbackStruct *cbs = (XmDrawingAreaCallbackStruct *)NULL;
  Position x,y;
  int region_found;
  
  ENTER(drawing_area_input);
  USE_PARAMETER(widget);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (cbs = (XmDrawingAreaCallbackStruct *)call_data)
    {
      if ((cbs->event->xany.type == ButtonRelease) &&
        (cbs->event->xbutton.button != 3))
      {
        x = cbs->event->xbutton.x;
        y = cbs->event->xbutton.y;
        current = cell->components;
        /* find the region the user clicked in */
        region_found = 0;
        while ((current != (struct Cell_component *)NULL) && !region_found)
        {
          if (XPointInRegion(current->region,x,y))
          {
            region_found = 1;
          }
          else
          {
            current = current->next;
          }
        } /* while ((current != ...) && !region_found) */
        if (region_found && (current != (struct Cell_component *)NULL))
        {
          bring_up_parameter_dialog(current);
        }
      }
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"drawing_area_input. "
      "Missing Cell window");
  }
  LEAVE;
} /* END drawing_area_input() */
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
static void drawing_area_expose(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 03 February 1999

DESCRIPTION :
Callback for expose events in the drawing area.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  XmDrawingAreaCallbackStruct *cbs = (XmDrawingAreaCallbackStruct *)NULL;
  
  ENTER(drawing_area_expose);
  USE_PARAMETER(widget);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (cbs = (XmDrawingAreaCallbackStruct *)call_data)
    {
      if ((cell->image_map).pixmap != (Pixmap)NULL)
      {
        /* the pixmap exists, so only need to re-paste it to the
        drawing area */
        copy_pixmap_to_window(cell->user_interface->display,
          (cell->image_map).pixmap,(cell->image_map).window,
          (cell->image_map).width,(cell->image_map).height);
      }
      else
      {
        /* need to create the image map's pixmap */
        if (create_image_map(cell,cbs))
        {
          /* then copy it to the window */
          copy_pixmap_to_window(cell->user_interface->display,
            (cell->image_map).pixmap,(cell->image_map).window,
            (cell->image_map).width,(cell->image_map).height);
        }
        else
        {
          display_message(ERROR_MESSAGE,"drawing_area_expose. "
            "Unable to create the image map");
        }
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"drawing_area_expose. "
        "Missing drawing area callback structure");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"drawing_area_expose. "
      "Missing Cell window");
  }
  LEAVE;
} /* END drawing_area_expose() */
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
static void drawing_area_resize(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 03 February 1999

DESCRIPTION :
Callback for resize events in the drawing area.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(drawing_area_resize);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    /* redraw the image map */
    if (draw_image_map(cell))
    {
      copy_pixmap_to_window(cell->user_interface->display,
        (cell->image_map).pixmap,(cell->image_map).window,
        (cell->image_map).width,(cell->image_map).height);
    }
    else
    {
      display_message(ERROR_MESSAGE,"drawing_area_resize. "
        "Unable to draw the image map");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"drawing_area_resize. "
      "Missing Cell window");
  }
  LEAVE;
} /* END drawing_area_resize() */
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
static int cell_3d_change_object_not_identifier(struct FE_node *node,
  void *cell_window)
/*******************************************************************************
LAST MODIFIED : 07 September 1999

DESCRIPTION :
Function called when a node is clicked on??
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  char *name;
  int return_code;

  ENTER(cell_3d_change_object_not_identifier);
  if (cell = (struct Cell_window *)cell_window)
  {
    GET_NAME(FE_node)(node,&name);
    return_code = bring_up_dialog_for_name(cell,name);
  }
  else
  {
    display_message(ERROR_MESSAGE,"cell_3d_change_object_not_identifier. "
      "Missing cell window");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* END cell_3d_change_object_not_identifier() */

static void cell_3d_node_group_change(
  struct MANAGER_MESSAGE(GROUP(FE_node)) *message,void *cell_window)
/*******************************************************************************
LAST MODIFIED : 07 September 1999

DESCRIPTION :
Node group manager change callback. Brings up the parameter dialog for the
appropriate mechanism/border/sub-space when added to the active group
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  char *group_name;
  
  ENTER(cell_3d_node_group_change);
  if (cell = (struct Cell_window *)cell_window)
  {
    switch (message->change)
		{
			case MANAGER_CHANGE_ALL(GROUP(FE_node)):
      {
        display_message(INFORMATION_MESSAGE,"cell_3d_node_group_change. "
          "MANAGER_CHANGE_ALL\n");
      } break;
			case MANAGER_CHANGE_OBJECT(GROUP(FE_node)):
      {
        display_message(INFORMATION_MESSAGE,"cell_3d_node_group_change. "
          "MANAGER_CHANGE_OBJECT\n");
      } break;
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(GROUP(FE_node)):
      {
        /* bring up the parameter dialog for the first node group in the
           active group */
        FOR_EACH_OBJECT_IN_GROUP(FE_node)(cell_3d_change_object_not_identifier,
          (void *)cell,message->object_changed);
        /* ?? remove the node groups from the active group ?? */
        /* this don't work!!! */
        /*REMOVE_ALL_OBJECTS_FROM_GROUP(FE_node)(message->object_changed);*/
      } break;
			case MANAGER_CHANGE_DELETE(GROUP(FE_node)):
      {
        display_message(INFORMATION_MESSAGE,"cell_3d_node_group_change. "
          "MANAGER_CHANGE_DELETE\n");
      } break;
			case MANAGER_CHANGE_ADD(GROUP(FE_node)):
      {
        display_message(INFORMATION_MESSAGE,"cell_3d_node_group_change. "
          "MANAGER_CHANGE_ADD\n");
      } break;
			case MANAGER_CHANGE_IDENTIFIER(GROUP(FE_node)):
      {
        display_message(INFORMATION_MESSAGE,"cell_3d_node_group_change. "
          "MANAGER_CHANGE_IDENTIFIER\n");
      } break;
		}
  }
  else
  {
    display_message(ERROR_MESSAGE,"cell_3d_node_group_change. "
      "Missing cell window");
  }
  LEAVE;
} /* END cell_3d_node_group_change() */

static void set_up_cell_3d_scene(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 06 September 1999

DESCRIPTION :
Set-up the scene viewer for Cell 3D.
==============================================================================*/
{
  struct Colour *colour;
  struct File_read_graphics_object_from_obj_data *obj_data;
  struct File_read_FE_node_group_data *node_data;
  char file_name[250];
  struct GT_object *graphics_object;
  
  ENTER(set_up_cell_3d_scene);

  /* read in the graphics object for the membrane glyph */
  ALLOCATE(obj_data,struct File_read_graphics_object_from_obj_data,1);
  obj_data->object_list=(cell->cell_3d).graphics_object_list;
  obj_data->graphical_material_manager=
    (cell->cell_3d).graphical_material_manager;
  obj_data->time = 0.0;
  obj_data->graphics_object_name = "membrane";
  obj_data->render_type = RENDER_TYPE_SHADED;
  sprintf(file_name,"/usr/people/nickerso/3d_cell/membrane.obj\0");
  file_read_voltex_graphics_object_from_obj(file_name,(void *)obj_data);
  DEALLOCATE(obj_data);
  /* grab the graphics object */
  graphics_object = FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)
    ("membrane",(cell->cell_3d).graphics_object_list);
  /* if the membrane is not already in the glyph list, add it */
  if (!(IS_OBJECT_IN_LIST(GT_object)(graphics_object,
    (cell->cell_3d).glyph_list)))
  {
    ADD_OBJECT_TO_LIST(GT_object)(graphics_object,(cell->cell_3d).glyph_list);
  }
  /* if the membrane material does not exist, create it (allows for the
     material to be specified by other thatn Cell) */
  /* leave as default material for now !! */
  /* read in the node group for the membrane */
  ALLOCATE(node_data,struct File_read_FE_node_group_data,1);
  node_data->fe_field_manager=(cell->cell_3d).fe_field_manager;
  node_data->element_group_manager=(cell->cell_3d).element_group_manager;
  node_data->node_manager=(cell->cell_3d).node_manager;
  node_data->node_group_manager=(cell->cell_3d).node_group_manager;
  node_data->data_group_manager=(cell->cell_3d).data_group_manager;
  sprintf(file_name,"/usr/people/nickerso/3d_cell/membrane.exnode\0");
  file_read_FE_node_group(file_name,(void *)node_data);
  DEALLOCATE(node_data);
  
  /* this would draw the membrane graphics object into the default scene */
  /*Scene_add_graphics_object((cell->cell_3d).default_scene,graphics_object,0,
    "membrane");*/

  /* ?? add the callback for the node groups ?? */
  (cell->cell_3d).node_group_callback_id=
    MANAGER_REGISTER(GROUP(FE_node))(cell_3d_node_group_change,
      (void *)cell,(cell->cell_3d).node_group_manager);
  
  
  /* set-up the view */
  /* *** */
  /* *** Check graphics/scene_viewer.h for functions *** */
  /* *** */
  /* set the background colour to black */
  colour = create_Colour(0.0,0.0,0.0);
  Scene_viewer_set_background_colour((cell->cell_3d).scene_viewer,colour);
  /* set the view */
  Scene_viewer_set_lookat_parameters_non_skew((cell->cell_3d).scene_viewer,
    -0.824591,44.3154,-0.0387581,-0.824591,-2.43508,-0.0387581,1,0,0);
  Scene_viewer_set_view_simple((cell->cell_3d).scene_viewer,
    0,0,0,15,43.412,100);
  Scene_viewer_set_input_mode((cell->cell_3d).scene_viewer,SCENE_VIEWER_SELECT);
  LEAVE;
} /* END set_up_cell_3d_scene() */
#endif /* defined (OLD_CODE) */

static void initialise_cell_3d_scene(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 08 September 1999

DESCRIPTION :
Initialise the scene viewer for Cell 3D.
==============================================================================*/
{
  struct Colour *colour;
  
  ENTER(intialise_cell_3d_scene);
  /* set-up the view */
  /* *** */
  /* *** Check graphics/scene_viewer.h for functions *** */
  /* *** */
  /* set the background colour to black */
  colour = create_Colour(0.0,0.0,0.0);
  Scene_viewer_set_background_colour((cell->cell_3d).scene_viewer,colour);
  /* set the view */
  Scene_viewer_set_lookat_parameters_non_skew((cell->cell_3d).scene_viewer,
    0.371067,8.52279,-63.3145,-0.884569,0.135409,0.298719,1,0,0);
  Scene_viewer_set_view_simple((cell->cell_3d).scene_viewer,
    0,0,0,15,43.412,100);
  Scene_viewer_set_input_mode((cell->cell_3d).scene_viewer,SCENE_VIEWER_SELECT);
  /* turn off the axis */
  Scene_set_axis_visibility((cell->cell_3d).scene,g_INVISIBLE);
  LEAVE;
} /* END intialise_cell_3d_scene() */

#if defined (CELL_USE_NODES)
static void apply_button_callback(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 September 1999

DESCRIPTION :
Callback function for the apply button, used to update the nodal fields for a
specified node.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  struct FE_node *node = (struct FE_node *)NULL;
  
  ENTER(apply_button_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (node = TEXT_CHOOSE_OBJECT_GET_OBJECT(FE_node)(
      (cell->distributed).node_chooser_widget))
    {
      if (!Cell_window_to_FE_node(cell,node))
      {
        display_message(ERROR_MESSAGE,"apply_button_callback. "
          "Unable to set the nodal values");
      }
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"apply_button_callback. "
      "Missing cell window");
  }
  LEAVE;
} /* END apply_button_callback() */
#endif /* defined (CELL_USE_NODES) */

static void apply_button_callback(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 June 2000

DESCRIPTION :
Callback function for the apply button, used to update the element point
fields for the currently selected element point, from the values in the cell
window.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(apply_button_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    if (!Cell_window_to_element_point(cell))
    {
      display_message(ERROR_MESSAGE,"apply_button_callback. "
        "Unable to set the element point values");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"apply_button_callback. "
      "Missing cell window");
  }
  LEAVE;
} /* END apply_button_callback() */

#if defined (CELL_USE_NODES)
static void reset_button_callback(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 September 1999

DESCRIPTION :
Callback function for the reset button, used to revert back to the nodal values
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  struct FE_node *node;
  
  ENTER(reset_button_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    /* re-update the cell window from the node */
    if (node = TEXT_CHOOSE_OBJECT_GET_OBJECT(FE_node)(
      (cell->distributed).node_chooser_widget))
    {
      XXXXXXXXXX
      cell_window_update_node((Widget)NULL,(void *)cell,(void *)node);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"reset_button_callback. "
      "Missing cell window");
  }
  LEAVE;
} /* END reset_button_callback() */
#endif /* defined (CELL_USE_NODES) */

static void reset_button_callback(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 June 2000

DESCRIPTION :
Callback function for the reset button, used to revert back to the element
point values from the currently selected element point.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(reset_button_callback);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    /* re-update the cell window from the element point */
    Cell_window_update_from_element_point(cell);
  }
  else
  {
    display_message(ERROR_MESSAGE,"reset_button_callback. "
      "Missing cell window");
  }
  LEAVE;
} /* END reset_button_callback() */

static void export_to_cmiss_files(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 08 June 2000

DESCRIPTION :
Callback for File | Write | Export to CMISS files
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(export_to_cmiss_files);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    bring_up_export_window(cell);
  }
  else
  {
    display_message(ERROR_MESSAGE,"export_to_cmiss_files. "
      "Missing cell window");
  }
  LEAVE;
} /* END export_to_cmiss_files() */

static void export_control_curves(Widget widget,XtPointer cell_window,
  XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 September 1999

DESCRIPTION :
Callback for File | Write | Export time variables
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
  
  ENTER(export_control_curves);
  USE_PARAMETER(widget);
  USE_PARAMETER(call_data);
  if (cell = (struct Cell_window *)cell_window)
  {
    bring_up_export_control_curves_dialog(cell);
  }
  else
  {
    display_message(ERROR_MESSAGE,"export_control_curves. "
      "Missing cell window");
  }
  LEAVE;
} /* END export_control_curves() */

static int Cell_window_calculate_xi(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 08 June 2000

DESCRIPTION :
Ensures xi is correct for the currently selected element point, if any.
==============================================================================*/
{
	int return_code;
	struct FE_element *element;

	ENTER(Cell_window_calculate_xi);
	if (cell)
	{
		if (element=(cell->distributed).element_point_identifier.element)
		{
			return_code=Xi_discretization_mode_get_element_point_xi(
				(cell->distributed).element_point_identifier.xi_discretization_mode,
				get_FE_element_dimension(element),
				(cell->distributed).element_point_identifier.number_in_xi,
				(cell->distributed).element_point_identifier.exact_xi,
				(cell->distributed).element_point_number,
				(cell->distributed).xi);
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cell_window_calculate_xi.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cell_window_calculate_xi */

static int Cell_window_refresh_element(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 08 June 2000

DESCRIPTION :
Updates the element shown in the chooser to match that for the current point.
==============================================================================*/
{
	int return_code;
	struct FE_element *element;
 
	ENTER(Cell_window_refresh_element);
	if (cell)
	{
		return_code=1;
		if (element=(cell->distributed).element_point_identifier.element)
		{
			TEXT_CHOOSE_OBJECT_SET_OBJECT(FE_element)(
				(cell->distributed).element_widget,element);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cell_window_refresh_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cell_window_refresh_element */

static int Cell_window_refresh_point_number_text(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 08 June 2000

DESCRIPTION :
Updates the point_number text field. If there is a current element point,
writes its number, otherwise N/A.
==============================================================================*/
{
	char temp_string[20];
	int return_code,is_sensitive;
 
	ENTER(Cell_window_refresh_point_number_text);
	if (cell)
	{
		return_code=1;
		if ((cell->distributed).element_point_identifier.element)
		{
			sprintf(temp_string,"%d",(cell->distributed).element_point_number);
			XmTextFieldSetString((cell->distributed).point_number_text,temp_string);
			is_sensitive=True;
		}
		else
		{
			XmTextFieldSetString((cell->distributed).point_number_text,"N/A");
			is_sensitive=False;
		}
		XtSetSensitive((cell->distributed).point_number_text,is_sensitive);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cell_window_refresh_point_number_text.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cell_window_refresh_point_number_text */

static int Cell_window_refresh_grid_value_text(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 08 June 2000

DESCRIPTION :
Updates the grid_value text field. If there is a current element point, writes
the field value, otherwise N/A.
==============================================================================*/
{
	char *value_string;
	int is_sensitive,return_code;
	struct Computed_field *grid_field;
	struct FE_element *top_level_element;
 
	ENTER(Cell_window_refresh_grid_value_text);
	if (cell)
	{
		return_code=1;
		top_level_element=(struct FE_element *)NULL;
		if (((cell->distributed).element_point_identifier.element)&&
			(grid_field=CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
				(cell->distributed).grid_field_widget)))
		{
			if (value_string=Computed_field_evaluate_as_string_in_element(
				grid_field,(cell->distributed).element_point_identifier.element,
				(cell->distributed).xi,top_level_element))
			{
				XmTextFieldSetString((cell->distributed).grid_value_text,
					value_string);
				DEALLOCATE(value_string);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cell_window_refresh_grid_value_text.  "
					"Could not evaluate field");
				XmTextFieldSetString((cell->distributed).grid_value_text,"ERROR");
			}
			Computed_field_clear_cache(grid_field);
			is_sensitive=True;
		}
		else
		{
			XmTextFieldSetString((cell->distributed).grid_value_text,"N/A");
			is_sensitive=False;
		}
		XtSetSensitive((cell->distributed).grid_value_text,is_sensitive);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cell_window_refresh_grid_value_text.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cell_window_refresh_grid_value_text */

static int Cell_window_refresh_chooser_widgets(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 08 June 2000

DESCRIPTION :
Fills the widgets for choosing the element point with the current values.
==============================================================================*/
{
	int return_code;
 
	ENTER(Cell_window_refresh_chooser_widgets);
	if (cell)
	{
		return_code=1;
		Cell_window_refresh_element(cell);
		Cell_window_refresh_point_number_text(cell);
		Cell_window_refresh_grid_value_text(cell);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cell_window_refresh_chooser_widgets.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cell_window_refresh_chooser_widgets */

static void Cell_window_element_point_ranges_selection_change(
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct Element_point_ranges_selection_changes *changes,
	void *cell_window_void)
/*******************************************************************************
LAST MODIFIED : 14 June 2000

DESCRIPTION :
Callback for change in the global element_point selection.
==============================================================================*/
{
	int start,stop;
	struct Element_point_ranges *element_point_ranges;
	struct Cell_window *cell;
	struct Multi_range *ranges;

	ENTER(Cell_window_element_point_ranges_selection_change);
	if (element_point_ranges_selection&&changes&&(cell=
		(struct Cell_window *)cell_window_void))
	{
		/* get the last selected element_point and put it in the cell window */
		if ((element_point_ranges=
			FIRST_OBJECT_IN_LIST_THAT(Element_point_ranges)(
				(LIST_CONDITIONAL_FUNCTION(Element_point_ranges) *)NULL,(void *)NULL,
				changes->newly_selected_element_point_ranges_list))||
			(element_point_ranges=
				FIND_BY_IDENTIFIER_IN_LIST(Element_point_ranges,identifier)(
					&((cell->distributed).element_point_identifier),
					Element_point_ranges_selection_get_element_point_ranges_list(
						element_point_ranges_selection)))||
			(element_point_ranges=
				FIRST_OBJECT_IN_LIST_THAT(Element_point_ranges)(
					(LIST_CONDITIONAL_FUNCTION(Element_point_ranges) *)NULL,(void *)NULL,
					Element_point_ranges_selection_get_element_point_ranges_list(
						element_point_ranges_selection))))
		{
			Element_point_ranges_get_identifier(element_point_ranges,
				&((cell->distributed).element_point_identifier));
			if ((ranges=Element_point_ranges_get_ranges(element_point_ranges))&&
				Multi_range_get_range(ranges,0,&start,&stop))
			{
				(cell->distributed).element_point_number=start;
			}
			else
			{
				(cell->distributed).element_point_number=0;
			}
      Cell_window_calculate_xi(cell);
			Cell_window_refresh_chooser_widgets(cell);
      Cell_window_update_from_element_point(cell);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cell_window_element_point_ranges_selection_change.  "
			"Invalid argument(s)");
	}
	LEAVE;
} /* Cell_window_element_point_ranges_selection_change */

static void Cell_window_update_grid_field(Widget widget,
	void *cell_window_void,void *grid_field_void)
/*******************************************************************************
LAST MODIFIED : 08 June 2000

DESCRIPTION :
Callback for change of grid field.
==============================================================================*/
{
	struct Cell_window *cell;

	ENTER(Cell_window_update_grid_field);
	USE_PARAMETER(widget);
	USE_PARAMETER(grid_field_void);
	if (cell = (struct Cell_window *)cell_window_void)
	{
		Cell_window_refresh_grid_value_text(cell);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cell_window_update_grid_field.  Invalid argument(s)");
	}
	LEAVE;
} /* Cell_window_update_grid_field */

static int Cell_window_get_grid(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 08 June 2000

DESCRIPTION :
If there is a grid field defined for the element, gets its discretization and
sets the Xi_discretization_mode to XI_DISCRETIZATION_CELL_CORNERS, otherwise
leaves the current discretization/mode intact.
==============================================================================*/
{
	int return_code;
	struct Computed_field *grid_field;
	struct FE_element *element;
	struct FE_field *grid_fe_field;

	ENTER(Cell_window_get_grid);
	if (cell)
	{
		return_code=1;
		element=(cell->distributed).element_point_identifier.element;
		if (element&&(grid_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
			Computed_field_is_scalar_integer_grid_in_element,(void *)element,
			Computed_field_package_get_computed_field_manager(
				cell->computed_field_package)))&&
			Computed_field_get_type_finite_element(grid_field,&grid_fe_field)&&
			FE_element_field_is_grid_based(element,grid_fe_field))
		{
			get_FE_element_field_grid_map_number_in_xi(element,grid_fe_field,
				(cell->distributed).element_point_identifier.number_in_xi);
			(cell->distributed).element_point_identifier.xi_discretization_mode=
				XI_DISCRETIZATION_CELL_CORNERS;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cell_window_get_grid.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cell_window_get_grid */

static int Cell_window_select_current_point(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 08 June 2000

DESCRIPTION :
Makes the currently described element point the only one in the global
selection. Does nothing if no current element point.
==============================================================================*/
{
	int return_code;
 	struct Element_point_ranges *element_point_ranges;

	ENTER(Cell_window_select_current_point);
	if (cell)
	{
		if ((cell->distributed).element_point_identifier.element)
		{
			if (element_point_ranges=CREATE(Element_point_ranges)(
				&((cell->distributed).element_point_identifier)))
			{
				Element_point_ranges_add_range(element_point_ranges,
					(cell->distributed).element_point_number,
					(cell->distributed).element_point_number);
				Element_point_ranges_selection_begin_cache(
					cell->element_point_ranges_selection);
				Element_point_ranges_selection_clear(
					cell->element_point_ranges_selection);
				return_code=
					Element_point_ranges_selection_select_element_point_ranges(
						cell->element_point_ranges_selection,
						element_point_ranges);
				Element_point_ranges_selection_end_cache(
					cell->element_point_ranges_selection);
				DESTROY(Element_point_ranges)(&element_point_ranges);
			}
			else
			{
				return_code=0;
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"Cell_window_select_current_point.  Failed");
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cell_window_select_current_point.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cell_window_select_current_point */

static void Cell_window_update_element(Widget widget,
	void *cell_window_void,void *element_void)
/*******************************************************************************
LAST MODIFIED : 08 June 2000

DESCRIPTION :
Callback for change of element.
==============================================================================*/
{
	struct Cell_window *cell;

	ENTER(Cell_window_update_element);
	USE_PARAMETER(widget);
	if (cell = (struct Cell_window *)cell_window_void)
	{
		(cell->distributed).element_point_identifier.element=
			(struct FE_element *)element_void;
		if ((cell->distributed).element_point_identifier.element)
		{
			if (XI_DISCRETIZATION_CELL_CORNERS==
				(cell->distributed).element_point_identifier.xi_discretization_mode)
			{
				Cell_window_get_grid(cell);
			}
			Cell_window_calculate_xi(cell);
			Cell_window_select_current_point(cell);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cell_window_update_element.  Invalid argument(s)");
	}
	LEAVE;
} /* Cell_window_update_element */

static void Cell_window_point_number_text_CB(Widget widget,
	void *cell_window_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 30 May 2000

DESCRIPTION :
Called when entry is made into the point_number_text field.
==============================================================================*/
{
	char *value_string;
	int element_point_number;
	struct Cell_window *cell;
	XmAnyCallbackStruct *any_callback;

	ENTER(Cell_window_point_number_text_CB);
	USE_PARAMETER(widget);
	if ((cell = (struct Cell_window *)cell_window_void)&&
		(any_callback = (XmAnyCallbackStruct *)call_data))
	{
		if (XmCR_ACTIVATE == any_callback->reason)
		{
			/* Get the text string */
			if (value_string =
				XmTextFieldGetString((cell->distributed).point_number_text))
			{
				if ((1==sscanf(value_string,"%d",&element_point_number))&&
					Element_point_ranges_identifier_element_point_number_is_valid(
						&((cell->distributed).element_point_identifier),
						element_point_number))
				{
					(cell->distributed).element_point_number=element_point_number;
					Cell_window_select_current_point(cell);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Cell_window_point_number_text_CB.  Invalid point number");
				}
				XtFree(value_string);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cell_window_point_number_text_CB.  Missing text");
			}
		}
		/* always restore point_number_text to actual value stored */
		Cell_window_refresh_point_number_text(cell);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cell_window_point_number_text_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Cell_window_point_number_text_CB */

static void Cell_window_grid_value_text_CB(Widget widget,
	void *cell_window_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 08 June 2000

DESCRIPTION :
Called when entry is made into the grid_value_text field.
==============================================================================*/
{
	char *value_string;
	int grid_value;
	struct Computed_field *grid_field;
	struct Cell_window *cell;
	struct FE_element_grid_to_Element_point_ranges_list_data grid_to_list_data;
	struct FE_field *grid_fe_field;
	XmAnyCallbackStruct *any_callback;

	ENTER(Cell_window_grid_value_text_CB);
	USE_PARAMETER(widget);
	if ((cell = (struct Cell_window *)cell_window_void)&&
		(any_callback=(XmAnyCallbackStruct *)call_data))
	{
		if (XmCR_ACTIVATE == any_callback->reason)
		{
			/* Get the text string */
			if (value_string=
				XmTextFieldGetString((cell->distributed).grid_value_text))
			{
				if ((grid_field=CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
					(cell->distributed).grid_field_widget))&&
					Computed_field_get_type_finite_element(grid_field,&grid_fe_field))
				{
					if (1==sscanf(value_string,"%d",&grid_value))
					{
						if ((grid_to_list_data.grid_value_ranges=CREATE(Multi_range)())&&
							Multi_range_add_range(grid_to_list_data.grid_value_ranges,
								grid_value,grid_value))
						{
							if (grid_to_list_data.element_point_ranges_list=
								CREATE(LIST(Element_point_ranges))())
							{
								grid_to_list_data.grid_fe_field=grid_fe_field;
								/* inefficient: go through every element in manager */
								FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
									FE_element_grid_to_Element_point_ranges_list,
									(void *)&grid_to_list_data,
									(cell->cell_3d).element_manager);
								if (0<NUMBER_IN_LIST(Element_point_ranges)(
									grid_to_list_data.element_point_ranges_list))
								{
									Element_point_ranges_selection_begin_cache(
										cell->element_point_ranges_selection);
									Element_point_ranges_selection_clear(
										cell->element_point_ranges_selection);
									FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
										Element_point_ranges_select,
										(void *)cell->element_point_ranges_selection,
										grid_to_list_data.element_point_ranges_list);
									Element_point_ranges_selection_end_cache(
										cell->element_point_ranges_selection);
								}
								DESTROY(LIST(Element_point_ranges))(
									&(grid_to_list_data.element_point_ranges_list));
							}
							DESTROY(Multi_range)(&(grid_to_list_data.grid_value_ranges));
						}
					}
				}
				XtFree(value_string);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cell_window_grid_value_text_CB.  Missing text");
			}
		}
		/* always restore grid_value_text to actual value stored */
		Cell_window_refresh_grid_value_text(cell);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cell_window_grid_value_text_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Cell_window_grid_value_text_CB */

/*
Global functions
================
*/
struct Cell_window *create_Cell_window(struct User_interface *user_interface,
  char *filename,struct MANAGER(Control_curve) *control_curve_manager,
  struct Unemap_package *package,
  struct Colour *background_colour,struct MANAGER(Light) *light_manager,
  struct Light *default_light,struct MANAGER(Light_model) *light_model_manager,
  struct Light_model *default_light_model,struct MANAGER(Scene) *scene_manager,
  struct Scene *default_scene,struct MANAGER(Texture) *texture_manager,
  struct LIST(GT_object) *graphics_object_list,
  struct MANAGER(Graphical_material) *graphical_material_manager,
  struct LIST(GT_object) *glyph_list,struct MANAGER(FE_field) *fe_field_manager,
  struct MANAGER(GROUP(FE_element)) *element_group_manager,
  struct MANAGER(FE_node) *node_manager,
	struct MANAGER(FE_element) *element_manager,
  struct MANAGER(GROUP(FE_node)) *node_group_manager,
  struct MANAGER(GROUP(FE_node)) *data_group_manager,
  struct Graphical_material *default_graphical_material,
  struct MANAGER(Spectrum) *spectrum_manager,struct Spectrum *default_spectrum,
  struct Computed_field_package *computed_field_package,
	struct Element_point_ranges_selection *element_point_ranges_selection,
  struct Execute_command *execute_command)
/*******************************************************************************
LAST MODIFIED : 9 June 2000

DESCRIPTION :
Create the structures and retrieve the cell window from the uil file. <filename>
specifies a file to print messages to, if non-NULL.
==============================================================================*/
{
  struct Cell_window *cell = (struct Cell_window *)NULL;
#if defined (MOTIF)
  struct File_open_data *variables_file_open_data,*variables_file_write_data;
  struct File_open_data *parameters_file_open_data;
  struct File_open_data *model_file_write_data,*cmiss_file_write_data;
  Atom WM_DELETE_WINDOW;
	struct Callback_data callback;
  struct Scene_input_callback scene_input_callback;
  int init_widgets = 0;
	MrmType cell_window_class;
  static MrmRegisterArg callback_list[] = {
    {"export_control_curves",(XtPointer)export_control_curves},
    {"identify_export_button",(XtPointer)identify_export_button},
    {"export_to_cmiss_files",(XtPointer)export_to_cmiss_files},
    {"apply_button_callback",(XtPointer)apply_button_callback},
    {"reset_button_callback",(XtPointer)reset_button_callback},
#if defined (CELL_USE_NODES)
    {"identify_node_chooser_form",(XtPointer)identify_node_chooser_form},
    {"identify_node_chooser_label",(XtPointer)identify_node_chooser_label},
#endif /* defined (CELL_USE_NODES) */
    {"identify_element_form",(XtPointer)identify_element_form},
    {"identify_point_number_text",(XtPointer)identify_point_number_text},
    {"point_number_text_CB",(XtPointer)Cell_window_point_number_text_CB},
    {"grid_value_text_CB",(XtPointer)Cell_window_grid_value_text_CB},
    {"identify_grid_field_form",(XtPointer)identify_grid_field_form},
    {"identify_grid_value_text",(XtPointer)identify_grid_value_text},
    {"identify_description_label",(XtPointer)identify_description_label},
    {"identify_apply_button",(XtPointer)identify_apply_button},
    {"identify_reset_button",(XtPointer)identify_reset_button},
    {"identify_output_pane",(XtPointer)identify_output_pane},
    {"save_toggle_changed_callback",(XtPointer)save_toggle_changed_callback},
    {"edit_dist_changed_callback",(XtPointer)edit_dist_changed_callback},
    {"debug_toggle_changed_callback",(XtPointer)debug_toggle_changed_callback},
    {"input_mode_callback",(XtPointer)input_mode_callback},
    {"identify_drawing_area",(XtPointer)identify_drawing_area},
    {"identify_cell_3d_form",(XtPointer)identify_cell_3d_form},
    {"file_open_and_read",(XtPointer)open_file_and_read},
    {"file_open_and_write",(XtPointer)open_file_and_write},
    {"open_model_callback",(XtPointer)open_model_callback},
    {"file_close_callback",(XtPointer)cell_window_close},
    {"calculate_button_callback",(XtPointer)calculate_button_callback},
    {"clear_button_callback",(XtPointer)clear_button_callback},
    {"model_button_callback",(XtPointer)model_button_callback},
    {"variables_button_callback",(XtPointer)variables_button_callback},
    {"currents_button_callback",(XtPointer)currents_button_callback},
    {"parameters_button_callback",(XtPointer)parameters_button_callback},
    {"help_index_callback",(XtPointer)help_index_callback},
    {"help_desc_param_callback",(XtPointer)help_describe_parameter_callback},
    {"help_desc_comp_callback",(XtPointer)help_describe_component_callback},
#if defined (OLD_CODE)
    {"drawing_area_input",(XtPointer)drawing_area_input},
    {"drawing_area_expose",(XtPointer)drawing_area_expose},
    {"drawing_area_resize",(XtPointer)drawing_area_resize}
#endif /* defined (OLD_CODE) */
    {"drawing_area_input",(XtPointer)NULL},
    {"drawing_area_expose",(XtPointer)NULL},
    {"drawing_area_resize",(XtPointer)NULL}
  }; /* callback_list */
  static MrmRegisterArg identifier_list[] = {
    {"cell_window_structure",(XtPointer)NULL},
    {"drawing_area_width",(XtPointer)NULL},
    {"drawing_area_height",(XtPointer)NULL},
    {"variables_file_open_data",(XtPointer)NULL},
    {"variables_file_write_data",(XtPointer)NULL},
    {"parameters_file_open_data",(XtPointer)NULL},
    {"parameters_file_write_data",(XtPointer)NULL},
    {"model_file_write_data",(XtPointer)NULL},
    {"cmiss_file_write_data",(XtPointer)NULL}
  }; /* identifier_list */
#define XmNdefaultBackground "defaultBackground"
#define XmCDefaultBackground "DefaultBackground"
#define XmNdefaultForeground "defaultForeground"
#define XmCDefaultForeground "DefaultForeground"
#define XmNchannelColour "channelColour"
#define XmCChannelColour "ChannelColour"
#define XmNchannelLabelColour "channelLabelColour"
#define XmCChannelLabelColour "ChannelLabelColour"
#define XmNpumpColour "pumpColour"
#define XmCPumpColour "PumpColour"
#define XmNpumpLabelColour "pumpLabelColour"
#define XmCPumpLabelColour "PumpLabelColour"
#define XmNexchangerColour "exchangerColour"
#define XmCExchangerColour "ExchangerColour"
#define XmNexchangerLabelColour "exchangerLabelColour"
#define XmCExchangerLabelColour "ExchangerLabelColour"
#define XmNmechanicalColour "mechanicalColour"
#define XmCMechanicalColour "MechanicalColour"
#define XmNmechanicalLabelColour "mechanicalLabelColour"
#define XmCMechanicalLabelColour "MechanicalLabelColour"
#define XmNionColour "ionColour"
#define XmCIonColour "IonColour"
#define XmNionLabelColour "ionLabelColour"
#define XmCIonLabelColour "IonLabelColour"
#define XmNcellLabelFont "cellLabelFont"
#define XmCCellLabelFont "CellLabelFont"
#define XmNtextEmphasisColour "textEmphasisColour"
#define XmCTextEmphasisColour "TextEmphasisColour"
  static XtResource resources[] = {
    {
      XmNdefaultBackground,
      XmCDefaultBackground,
      XmRString,
      sizeof(char *),
      XtOffsetOf(User_settings,default_background),
      XmRString,
      "black"
    },
    {
      XmNdefaultForeground,
      XmCDefaultForeground,
      XmRString,
      sizeof(char *),
      XtOffsetOf(User_settings,default_foreground),
      XmRString,
      "yellow"
    },
    {
      XmNchannelColour,
      XmCChannelColour,
      XmRString,
      sizeof(char *),
      XtOffsetOf(User_settings,channel_colour),
      XmRString,
      "blue"
    },
    {
      XmNchannelLabelColour,
      XmCChannelLabelColour,
      XmRString,
      sizeof(char *),
      XtOffsetOf(User_settings,channel_label_colour),
      XmRString,
      "SkyBlue"
    },
    {
      XmNexchangerColour,
      XmCExchangerColour,
      XmRString,
      sizeof(char *),
      XtOffsetOf(User_settings,exchanger_colour),
      XmRString,
      "firebrick"
    },
    {
      XmNexchangerLabelColour,
      XmCExchangerLabelColour,
      XmRString,
      sizeof(char *),
      XtOffsetOf(User_settings,exchanger_label_colour),
      XmRString,
      "red"
    },
    {
      XmNpumpColour,
      XmCPumpColour,
      XmRString,
      sizeof(char *),
      XtOffsetOf(User_settings,pump_colour),
      XmRString,
      "DarkGreen"
    },
    {
      XmNpumpLabelColour,
      XmCPumpLabelColour,
      XmRString,
      sizeof(char *),
      XtOffsetOf(User_settings,pump_label_colour),
      XmRString,
      "green"
    },
    {
      XmNmechanicalColour,
      XmCMechanicalColour,
      XmRString,
      sizeof(char *),
      XtOffsetOf(User_settings,mech_colour),
      XmRString,
      "RoyalBlue"
    },
    {
      XmNmechanicalLabelColour,
      XmCMechanicalLabelColour,
      XmRString,
      sizeof(char *),
      XtOffsetOf(User_settings,mech_label_colour),
      XmRString,
      "SkyBlue"
    },
    {
      XmNionColour,
      XmCIonColour,
      XmRString,
      sizeof(char *),
      XtOffsetOf(User_settings,ion_colour),
      XmRString,
      "lightGoldenRod"
    },
    {
      XmNionLabelColour,
      XmCIonLabelColour,
      XmRString,
      sizeof(char *),
      XtOffsetOf(User_settings,ion_label_colour),
      XmRString,
      "DarkViolet"
    },
    {
      XmNcellLabelFont,
      XmCCellLabelFont,
      XmRString,
      sizeof(char *),
      XtOffsetOf(User_settings,label_font),
      XmRString,
      "*-times-medium-r-normal--12-*"
    },
    {
      XmNtextEmphasisColour,
      XmCTextEmphasisColour,
      XmRPixel,
      sizeof(Pixel),
      XtOffsetOf(User_settings,text_emphasis_colour),
      XmRString,
      "red"
    }
  }; /* resources */
#endif /* if defined (MOTIF) */
  unsigned int width,height;
#if defined (CELL_USE_NODES)
  struct FE_node *node = (struct FE_node *)NULL;
#endif /* defined (CELL_USE_NODES) */
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Element_point_ranges *element_point_ranges;
	struct Computed_field *grid_field;
	struct Multi_range *ranges;
  int i,number_of_faces,start,stop;
  
  ENTER(create_Cell_window);
  USE_PARAMETER(init_widgets);
  if (user_interface)
  {
#if defined (MOTIF)
    if (MrmOpenHierarchy_base64_string(cell_window_uidh,&cell_window_hierarchy,
			&cell_window_hierarchy_open))
    {
#endif /* defined (MOTIF) */
      if (ALLOCATE(cell,struct Cell_window,1))
      {
        /* set the output file */
        if (filename)
        {
          cell->output_file = fopen(filename,"w");
        }
        else
        {
          cell->output_file = (FILE *)NULL;
        }
        cell->user_interface = user_interface;
        cell->single_cell = 0;
        cell->execute_command = execute_command;
        cell->computed_field_package = computed_field_package;
        cell->element_point_ranges_selection = element_point_ranges_selection;
        (cell->unemap).package = package;
#if defined(MOTIF)
        cell->default_values = 0;
        cell->current_model = (char *)NULL;
        cell->current_model_id = (char *)NULL;
        cell->window = (Widget)NULL;
        cell->shell = (Widget)NULL;
        cell->output_pane = (Widget)NULL;
        /* initialise the menu */
        (cell->menu).save = 0;
        (cell->menu).debug = 0;
        /* initialise the image map */
        (cell->image_map).drawing_area = (Widget)NULL;
        (cell->image_map).pixmap = (Pixmap)NULL;
        (cell->image_map).window = (Window)NULL;
        (cell->image_map).width = 0;
        (cell->image_map).height = 0;
        /* initialise the UnEMAP stuff */
        (cell->unemap).analysis.window_shell = (Widget)NULL;
        (cell->unemap).analysis.activation = (Widget)NULL;
        (cell->unemap).analysis.window = (struct Analysis_window *)NULL;
        (cell->unemap).analysis.trace = (struct Trace_window *)NULL;
        (cell->unemap).analysis.mapping_work_area =
          (struct Mapping_work_area *)NULL;
        (cell->unemap).analysis.mapping_window = (struct Mapping_window *)NULL;
        (cell->unemap).analysis.rig = (struct Rig *)NULL;
        (cell->unemap).analysis.highlight = (struct Device **)NULL;
        (cell->unemap).analysis.datum = 0;
        (cell->unemap).analysis.detection = EDA_INTERVAL;
        (cell->unemap).analysis.datum_type = AUTOMATIC_DATUM;
        (cell->unemap).analysis.edit_order = DEVICE_ORDER;
        (cell->unemap).analysis.signal_order = CHANNEL_ORDER;
        (cell->unemap).analysis.calculate_events = 0;
        (cell->unemap).analysis.number_of_events = 1;
        (cell->unemap).analysis.event_number = 1;
        (cell->unemap).analysis.threshold = 90;
        (cell->unemap).analysis.minimum_separation = 100;
        (cell->unemap).analysis.map_type = NO_MAP_FIELD;
        (cell->unemap).analysis.bard_signal_file_data =
          (struct File_open_data *)NULL;
        (cell->unemap).number_of_saved_buffers = 0;
        (cell->unemap).saved_buffers = (struct Signal_buffer **)NULL;
        /* initialise the time step information */
        (cell->time_steps).TABT = (float *)NULL;
        (cell->time_steps).TSTART = (float *)NULL;
        (cell->time_steps).TEND = (float *)NULL;
        (cell->time_steps).DT = 0.0;
        /* initialise time variable */
        (cell->control_curve).control_curve_manager = control_curve_manager;
        (cell->control_curve).control_curve_editor_dialog = (Widget)NULL;
        /* initialise Cell 3D */
        (cell->cell_3d).form = (Widget)NULL;
        (cell->cell_3d).node_group_callback_id=(void *)NULL;
        (cell->cell_3d).background_colour=background_colour;
        (cell->cell_3d).light_manager=light_manager;
        (cell->cell_3d).default_light=default_light;
        (cell->cell_3d).light_model_manager=light_model_manager;
        (cell->cell_3d).default_light_model=default_light_model;
        (cell->cell_3d).scene_manager=scene_manager;
        (cell->cell_3d).texture_manager=texture_manager;
        (cell->cell_3d).graphics_object_list=graphics_object_list;
        (cell->cell_3d).graphical_material_manager=graphical_material_manager;
        (cell->cell_3d).default_graphical_material=default_graphical_material;
        (cell->cell_3d).glyph_list = glyph_list;
        (cell->cell_3d).fe_field_manager=fe_field_manager;
        (cell->cell_3d).element_group_manager=element_group_manager;
        (cell->cell_3d).node_manager=node_manager;
        (cell->cell_3d).element_manager=element_manager;
        (cell->cell_3d).node_group_manager=node_group_manager;
        (cell->cell_3d).data_group_manager=data_group_manager;
        (cell->cell_3d).spectrum_manager=spectrum_manager;
        (cell->cell_3d).default_spectrum=default_spectrum;
        if ((cell->cell_3d).scene = CREATE(Scene)("Cell 3D"))
        {
          Scene_enable_graphics((cell->cell_3d).scene,
            (cell->cell_3d).glyph_list,
            (cell->cell_3d).graphical_material_manager,
            (cell->cell_3d).default_graphical_material,
            (cell->cell_3d).light_manager,
            (cell->cell_3d).spectrum_manager,
            (cell->cell_3d).default_spectrum,
            (cell->cell_3d).texture_manager);
          Scene_disable_time_behaviour((cell->cell_3d).scene);
          if (!ADD_OBJECT_TO_MANAGER(Scene)((cell->cell_3d).scene,
            (cell->cell_3d).scene_manager))
          {
            display_message(WARNING_MESSAGE,"create_Cell_window. "
              "Unable to add scene to manager, using default");
            DESTROY(Scene)(&(cell->cell_3d).scene);
            (cell->cell_3d).scene = default_scene;
          }
        }
        else
        {
          display_message(WARNING_MESSAGE,"create_Cell_window. "
            "Unable to create a new scene, using default");
          (cell->cell_3d).scene = default_scene;
        }
        /* initialise distributed widgets/structures */
#if defined (CELL_USE_NODES)
        (cell->distributed).edit = 0; /* always start with false */
        (cell->distributed).node_chooser_label = (Widget)NULL;
        (cell->distributed).node_chooser_form = (Widget)NULL;
        (cell->distributed).node_chooser_widget = (Widget)NULL;
        (cell->distributed).description_label = (Widget)NULL;
        (cell->distributed).apply_button = (Widget)NULL;
        (cell->distributed).reset_button = (Widget)NULL;
#endif /* defined (CELL_USE_NODES) */
        (cell->distributed).export_menu_button = (Widget)NULL;
        (cell->distributed).element_point_identifier.element=
					(struct FE_element *)NULL;
        (cell->distributed).element_point_identifier.top_level_element=
					(struct FE_element *)NULL;
				(cell->distributed).element_copy=(struct FE_element *)NULL;
				(cell->distributed).element_point_identifier.xi_discretization_mode=
					XI_DISCRETIZATION_EXACT_XI;
				for (i=0;i<MAXIMUM_ELEMENT_XI_DIMENSIONS;i++)
				{
					(cell->distributed).element_point_identifier.number_in_xi[i]=1;
					(cell->distributed).xi[i]=
						(cell->distributed).element_point_identifier.exact_xi[i]=0.5;
				}
				(cell->distributed).element_point_number=0;
        (cell->distributed).element_form=(Widget)NULL;
				(cell->distributed).element_widget=(Widget)NULL;
				(cell->distributed).point_number_text=(Widget)NULL;
				(cell->distributed).grid_field_form=(Widget)NULL;
				(cell->distributed).grid_field_widget=(Widget)NULL;
				(cell->distributed).grid_value_text=(Widget)NULL;
        (cell->distributed).edit = 0; /* always start with false */
        (cell->distributed).description_label = (Widget)NULL;
        (cell->distributed).apply_button = (Widget)NULL;
        (cell->distributed).reset_button = (Widget)NULL;
        (cell->distributed).export_menu_button = (Widget)NULL;
        /* initialise other structures */
        cell->user_settings = (struct Cell_user_settings *)NULL;
        cell->model_dialog = (struct Model_dialog *)NULL;
        cell->variables_dialog = (struct Variables_dialog *)NULL;
        cell->variables = (struct Cell_variable *)NULL;
        cell->components = (struct Cell_component *)NULL;
        cell->parameters = (struct Cell_parameter *)NULL;
        cell->outputs = (struct Cell_output *)NULL;
        cell->graphics = (struct Cell_graphic *)NULL;
        cell->export_dialog = (struct Export_dialog *)NULL;
        cell->export_control_curve_dialog = 
					(struct Export_control_curve_dialog *)NULL;
        /* create the cell window shell */
        if (cell->shell = XtVaCreatePopupShell("cell_window_shell",
          xmDialogShellWidgetClass,user_interface->application_shell,
					XmNdeleteResponse,XmDO_NOTHING,
          XmNmwmDecorations,MWM_DECOR_ALL,
          XmNmwmFunctions,MWM_FUNC_ALL,
          XmNtransient,FALSE,
          NULL))
        {
          /* identify the shell for the busy icon */
          create_Shell_list_item(&(cell->shell),user_interface);
          /* add the destroy callback */
          /*XtAddCallback(cell->shell,XmNdestroyCallback,destroy_Cell_window,
            (XtPointer)cell);*/
          XtAddCallback(cell->shell,XmNdestroyCallback,cell_window_close,
            (XtPointer)cell);
          WM_DELETE_WINDOW=XmInternAtom(XtDisplay(cell->shell),
            "WM_DELETE_WINDOW",FALSE);
          XmAddWMProtocolCallback(cell->shell,WM_DELETE_WINDOW,
            cell_window_close,(XtPointer)cell);
          /* register cellbacks in UIL */
          if (MrmSUCCESS == MrmRegisterNamesInHierarchy(cell_window_hierarchy,
            callback_list,XtNumber(callback_list)))
          {
            /* set the identifier's values */
            width = 2*user_interface->screen_width/3;
            height = 2*user_interface->screen_height/3;
            identifier_list[0].value = (XtPointer)cell;
            identifier_list[1].value = (XtPointer)width;
            identifier_list[2].value = (XtPointer)height;
#if defined (OLD_CODE)
            if (variables_file_open_data = create_File_open_data(".variables",
              REGULAR,read_variables_file,(XtPointer)cell,0,user_interface))
#endif /* defined (OLD_CODE) */
            if (variables_file_open_data = create_File_open_data(".variables",
              REGULAR,NULL,(XtPointer)cell,0,user_interface))
            {
              identifier_list[3].value = (XtPointer)variables_file_open_data;
            }
            else
            {
              identifier_list[3].value = (XtPointer)NULL;
            }
#if defined (OLD_CODE)
            if (variables_file_write_data = create_File_open_data(".variables",
              REGULAR,write_variables_file,(XtPointer)cell,0,user_interface))
#endif /* defined (OLD_CODE) */
            if (variables_file_write_data = create_File_open_data(".variables",
              REGULAR,NULL,(XtPointer)cell,0,user_interface))
            {
              identifier_list[4].value = (XtPointer)variables_file_write_data;
            }
            else
            {
              identifier_list[4].value = (XtPointer)NULL;
            }
#if defined (OLD_CODE)
            if (parameters_file_open_data = create_File_open_data(".parameters",
              REGULAR,read_parameters_file,(XtPointer)cell,0,user_interface))
#endif /* defined (OLD_CODE) */
            if (parameters_file_open_data = create_File_open_data(".parameters",
              REGULAR,NULL,(XtPointer)cell,0,user_interface))
            {
              identifier_list[5].value = (XtPointer)parameters_file_open_data;
            }
            else
            {
              identifier_list[5].value = (XtPointer)NULL;
            }
#if 0
            if (parameters_file_write_data =
              create_File_open_data(".parameters",
                REGULAR,write_parameters_file,(XtPointer)cell,0,user_interface))
            {
              identifier_list[6].value = (XtPointer)parameters_file_write_data;
            }
            else
            {
              identifier_list[6].value = (XtPointer)NULL;
            }
#else /* if 0 */
            identifier_list[6].value = (XtPointer)NULL;
#endif
            if (model_file_write_data = create_File_open_data(".model",
              REGULAR,write_model_file,(XtPointer)cell,0,user_interface))
            {
              identifier_list[7].value = (XtPointer)model_file_write_data;
            }
            else
            {
              identifier_list[7].value = (XtPointer)NULL;
            }
            if (cmiss_file_write_data = create_File_open_data(".ipcell",
              REGULAR,write_cmiss_file,(XtPointer)cell,0,user_interface))
            {
              identifier_list[8].value = (XtPointer)cmiss_file_write_data;
            }
            else
            {
              identifier_list[8].value = (XtPointer)NULL;
            }
            /* register the identifiers */
            if (MrmSUCCESS == MrmRegisterNamesInHierarchy(cell_window_hierarchy,
              identifier_list,XtNumber(identifier_list)))
            {
              /* fetch the window widget */
              if (MrmSUCCESS == MrmFetchWidget(cell_window_hierarchy,
                "cell_window",cell->shell,&(cell->window),&cell_window_class))
              {
                init_widgets=1;
#if defined (CELL_USE_NODES)
                if ((cell->distributed).node_chooser_widget=
                  CREATE_TEXT_CHOOSE_OBJECT_WIDGET(FE_node)(
                    (cell->distributed).node_chooser_form,
                    (struct FE_node *)NULL,node_manager,
                    (MANAGER_CONDITIONAL_FUNCTION(FE_node) *)NULL,(void *)NULL,
                    GET_NAME(FE_node),node_string_to_FE_node))
                {
									callback.data=(void *)cell;
									callback.procedure=cell_window_update_node;
									TEXT_CHOOSE_OBJECT_SET_CALLBACK(FE_node)(
										(cell->distributed).node_chooser_widget,&callback);
									if (node=TEXT_CHOOSE_OBJECT_GET_OBJECT(FE_node)(
                    (cell->distributed).node_chooser_widget))
                  {
                    cell_window_update_node((Widget)NULL,(void *)cell,
                      (void *)node);
                  }
#if defined (NEW_CODE)
                  /* demo */
									node=TEXT_CHOOSE_OBJECT_GET_OBJECT(FE_node)(
                    (cell->distributed).node_chooser_widget);
									TEXT_CHOOSE_OBJECT_SET_OBJECT(FE_node)(
										(cell->distributed).node_chooser_widget,node);
#endif /* defined (NEW_CODE) */
#endif /* defined (CELL_USE_NODES) */

                  /* New Stuff - From element/element_point_viewer.c
                   *
                   * Create the widgets/structures for selecting element (grid)
                   * points.
                   */
                  if (element_point_ranges=
                    FIRST_OBJECT_IN_LIST_THAT(Element_point_ranges)
                    ((LIST_CONDITIONAL_FUNCTION(Element_point_ranges) *)NULL,
                      (void *)NULL,
                      Element_point_ranges_selection_get_element_point_ranges_list(
                        element_point_ranges_selection)))
                  {
                    Element_point_ranges_get_identifier(element_point_ranges,
                      &((cell->distributed).element_point_identifier));
                    if ((ranges=Element_point_ranges_get_ranges(
                      element_point_ranges))&&Multi_range_get_range(ranges,0,
                        &start,&stop))
                    {
                      (cell->distributed).element_point_number=start;
                    }
                  }
                  else
                  {
                    /* try to get any point in the first top-level element
										 * we can find.
										 */
										if ((cell->distributed).element_point_identifier.element=
											FIRST_OBJECT_IN_MANAGER_THAT(FE_element)(
												FE_element_is_top_level,(void *)NULL,element_manager))
										{
											(cell->distributed).element_point_identifier.top_level_element=
												(cell->distributed).element_point_identifier.element;
										}
										else
										{
											(cell->distributed).element_point_identifier.top_level_element=
												(struct FE_element *)NULL;
										}
										/* try to get a grid point, if possible */
										Cell_window_get_grid(cell);
                    /* make the element point the only one in the global
                     * selection
                     */
                    Cell_window_select_current_point(cell);
                  }
                  Cell_window_calculate_xi(cell);
                  if ((cell->distributed).element_point_identifier.top_level_element)
                  {
                    if ((cell->distributed).element_copy=ACCESS(FE_element)(
                      CREATE(FE_element)((cell->distributed).
                        element_point_identifier.top_level_element->identifier,
                        (cell->distributed).
												element_point_identifier.top_level_element)))
										{
											/* clear the faces of element_copy as messes up exterior
												 calculations for graphics created from them */
											number_of_faces=
												(cell->distributed).element_copy->shape->number_of_faces;
											for (i=0;i<number_of_faces;i++)
											{
												set_FE_element_face((cell->distributed).element_copy,i,
													(struct FE_element *)NULL);
											}
										}
                  }
                  else
                  {
                    (cell->distributed).element_copy=(struct FE_element *)NULL;
                  }
                  /* get callbacks from global element_point selection */
                  Element_point_ranges_selection_add_callback(
                    element_point_ranges_selection,
                    Cell_window_element_point_ranges_selection_change,
                    (void *)cell);
                  /* create the element chooser */
                  if (!((cell->distributed).element_widget=
                    CREATE_TEXT_CHOOSE_OBJECT_WIDGET(FE_element)(
                      (cell->distributed).element_form,
                      (cell->distributed).element_point_identifier.element,
                      element_manager,
                      (MANAGER_CONDITIONAL_FUNCTION(FE_element) *)NULL,
                      (void *)NULL,
                      FE_element_to_any_element_string,
                      any_element_string_to_FE_element)))
                  {
                    init_widgets=0;
                  }
                  /* set the initial grid field to use for selecting
                   * element points - if a grid_point_number field exists
                   * use that, otherwise just take the first one which is
                   * suitable
                   */
                  computed_field_manager =
                    Computed_field_package_get_computed_field_manager(
                      computed_field_package);
                  if (!(grid_field=
                    FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
                      "grid_point_number",computed_field_manager)))
                  {
                    grid_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
                      Computed_field_is_scalar_integer,(void *)NULL,
                      computed_field_manager);
                  }
                  /* create the grid field chooser for setting the field
                   * used to select element points
                   */
                  if (!((cell->distributed).grid_field_widget=
                    CREATE_CHOOSE_OBJECT_WIDGET(Computed_field)(
                      (cell->distributed).grid_field_form,grid_field,
                      computed_field_manager,Computed_field_is_scalar_integer,
                      (void *)(cell->distributed).element_copy)))
                  {
                    init_widgets=0;
                  }
                  if (init_widgets)
                  {
                    /* set the callback for changing the grid field */
                    callback.data=(void *)cell;
                    callback.procedure=Cell_window_update_grid_field;
                    CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
                      (cell->distributed).grid_field_widget,&callback);
                    /* set the callback for changing the element */
                    callback.procedure=Cell_window_update_element;
                    TEXT_CHOOSE_OBJECT_SET_CALLBACK(FE_element)(
                      (cell->distributed).element_widget,&callback);
                    /* update the widgets ?? */
                    Cell_window_refresh_chooser_widgets(cell);
                    XtSetSensitive((cell->distributed).point_number_text,False);
                    XtSetSensitive((cell->distributed).grid_value_text,False);
                    XtSetSensitive((cell->distributed).element_widget,False);
                    XtSetSensitive((cell->distributed).grid_field_widget,False);
                  }
                  else
                  {
                    display_message(ERROR_MESSAGE,
                      "create_Cell_window.  Could not init widgets");
                    DEALLOCATE(cell);
                    cell = (struct Cell_window *)NULL;
                  }
#if defined (CELL_USE_NODES)
                }
                else
                {
                  init_widgets=0;
                }
#endif /* defined (CELL_USE_NODES) */
                /* retrieve settings */
                if (ALLOCATE(cell->user_settings,User_settings,1))
                {
                  XtVaGetApplicationResources(cell->window,
                    (XtPointer)cell->user_settings,resources,
                    XtNumber(resources),NULL);
                  /* create the scene viewer for Cell 3D */
                  if ((cell->cell_3d).scene_viewer =
                    CREATE(Scene_viewer)((cell->cell_3d).form,
                      (cell->cell_3d).background_colour,
                      SCENE_VIEWER_DOUBLE_BUFFER,(cell->cell_3d).light_manager,
                      (cell->cell_3d).default_light,
                      (cell->cell_3d).light_model_manager,
                      (cell->cell_3d).default_light_model,
                      (cell->cell_3d).scene_manager,
                      (cell->cell_3d).scene,
                      (cell->cell_3d).texture_manager,cell->user_interface))
                  {
                    /* set-up the scene */
                    initialise_cell_3d_scene(cell);
#if defined (CELL_USE_NODES)
                    /* ?? add the callback for the node groups ?? */
                    (cell->cell_3d).node_group_callback_id=
                      MANAGER_REGISTER(GROUP(FE_node))(cell_node_group_change,
                        (void *)cell,(cell->cell_3d).node_group_manager);
#endif /* defined (CELL_USE_NODES) */
                    /* add the callback for the object picking */
                    scene_input_callback.procedure = cell_3d_picking_callback;
                    scene_input_callback.data = (void *)cell;
                    Scene_set_input_callback((cell->cell_3d).scene,
                      &scene_input_callback);
#if defined (NEW_CODE)
                    /* if I ever destroy the cell window need to remove the
                       callback */
                    scene_input_callback.procedure =
                      (Scene_input_callback_procedure *)NULL;
                    scene_input_callback.data = (void *)NULL;
                    Scene_set_input_callback(scene,&scene_input_callback);
#endif /* defined (NEW_CODE) */
                    XtManageChild(cell->window);
                    XtRealizeWidget(cell->shell);
                    XtPopup(cell->shell,XtGrabNone);
                  }
                  else
                  {
                    display_message(ERROR_MESSAGE,"create_Cell_window. "
                      "Unable to create the Cell 3D scene viewer");
                    DEALLOCATE(cell);
                    cell = (struct Cell_window *)NULL;
                  }
                }
                else
                {
                  display_message(ERROR_MESSAGE,"create_Cell_window. "
                    "Unable to allocate memory for the user settings");
                  DEALLOCATE(cell);
                  cell = (struct Cell_window *)NULL;
                }
              }
              else
              {
                display_message(ERROR_MESSAGE,"create_Cell_window. "
                  "Unable to fetch the window widget");
                DEALLOCATE(cell);
                cell = (struct Cell_window *)NULL;
              }
            }
            else
            {
              display_message(ERROR_MESSAGE,"create_Cell_window. "
                "Unable to register identifiers");
              DEALLOCATE(cell);
              cell = (struct Cell_window *)NULL;
            }
          }
          else
          {
            display_message(ERROR_MESSAGE,"create_Cell_window. "
              "Unable to register callbacks");
            DEALLOCATE(cell);
            cell = (struct Cell_window *)NULL;
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"create_Cell_window. "
            "Unable to create the cell window shell");
          DEALLOCATE(cell);
          cell = (struct Cell_window *)NULL;
        }
#endif /* defined (MOTIF) */
      }
      else
      {
        display_message(ERROR_MESSAGE,"create_Cell_window. "
          "Unable to allocate memory for the Cell_window structure");
        cell = (struct Cell_window *)NULL;
      }
#if defined (MOTIF)
    } /* if open hierarchy */
#endif /* defined (MOTIF) */
  }
  else
  {
    display_message(ERROR_MESSAGE,"create_Cell_window. "
      "Missing user interface");
    cell = (struct Cell_window *)NULL;
  }
  LEAVE;
  return(cell);
} /* END create_Cell_window() */

int write_cell_window(char *message,struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 03 February 1999

DESCRIPTION :
Writes the <message> to the <cell> window.
==============================================================================*/
{
  int return_code = 0;
  XmTextPosition text_pos;

  ENTER(write_cell_window);
  if (cell && cell->output_pane)
  {
    text_pos = XmTextGetLastPosition(cell->output_pane);
    XmTextInsert(cell->output_pane,text_pos,message);
    text_pos = XmTextGetLastPosition(cell->output_pane);
    XmTextShowPosition(cell->output_pane,text_pos);
    return_code = 1;
  }
  else
  {
    /* missing window */
    printf("%s",message);
    return_code = 1;
  }
  LEAVE;
  return(return_code);
} /* END write_cell_window() */

#if defined (CELL_USE_NODES)
void update_cell_window_from_node(struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 15 September 1999

DESCRIPTION :
Updates the cell window from a node, if a node exists in the node chooser
==============================================================================*/
{
  struct FE_node *node = (struct FE_node *)NULL;

  ENTER(update_cell_window_from_node);
  if (cell != (struct Cell_window *)NULL)
  {
    if (node=TEXT_CHOOSE_OBJECT_GET_OBJECT(FE_node)(
      (cell->distributed).node_chooser_widget))
    {
      ???XXXXXXXXXX????
      cell_window_update_node((Widget)NULL,(void *)cell,(void *)node);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"update_cell_window_from_node. "
      "Invalid arguments");
  }
  LEAVE;
} /* END update_cell_window_from_node() */
#endif /* defined (CELL_USE_NODES) */

int Cell_read_model(char *filename,struct Cell_window *cell)
/*******************************************************************************
LAST MODIFIED : 08 June 2000

DESCRIPTION :
Wrapper function used to execute the CELL READ MODEL command. If a file name
is specified the file is parsed, otherwise, the models dialog is popped-up.
==============================================================================*/
{
  int return_code = 0;
  
  ENTER(Cell_read_model);
  if (cell)
  {
    if (filename)
    {
      display_message(INFORMATION_MESSAGE,"Cell_read_model. "
        "This don't work yet!!\n"
        "  Need to do something like \"cell read/define models\" to define "
        "the models from the default \n    file given in the resource file.\n"
        "  Then can do a \"cell read model <filename>\" or maybe "
        "\"cell read model <name>\", where \n    name is defined by the "
        "models file already read in!!??????\n");
    }
    else
    {
      return_code = bring_up_model_dialog(cell);
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Cell_read_model. "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_read_model */
