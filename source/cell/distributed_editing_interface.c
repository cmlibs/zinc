/*******************************************************************************
FILE : distributed_editing_interface.c

LAST MODIFIED : 11 June 2001

DESCRIPTION :
The interface routines for editing distributed cellular parameters. Only used
when building Cell with CELL_DISTRIBUTED defined.
==============================================================================*/

#if defined (CELL_DISTRIBUTED)

#include <Xm/TextF.h>
#include <stdio.h>
#include <stdlib.h>
#include "cell/cell_variable.h"
#include "cell/distributed_editing_dialog.h"
#include "cell/distributed_editing_interface.h"
#include "choose/choose_element_group.h"
#include "choose/choose_computed_field.h"
#include "choose/text_choose_fe_element.h"
#include "computed_field/computed_field_finite_element.h"
#include "finite_element/finite_element_to_graphics_object.h"

/*
Module types
============
*/
struct Iterator_data
/*******************************************************************************
LAST MODIFIED : 17 January 2001

DESCRIPTION :
User data for iterating through the cell variables.
==============================================================================*/
{
  struct Cell_cmgui_interface *cmgui_interface;
  int comp_no;
  struct FE_element *element;
  FE_value *xi;
  struct FE_element *top_level_element;
}; /* struct Iterator_data */

struct Iterator_data_set
/*******************************************************************************
LAST MODIFIED : 24 January 2001

DESCRIPTION :
User data for iterating through the cell variables when setting field values.
==============================================================================*/
{
  struct Distributed_editing_interface *interface;
  struct Element_point_ranges_set_grid_values_data *set_grid_values_data;
}; /* struct Iterator_data_set */

struct Distributed_editing_interface
/*******************************************************************************
LAST MODIFIED : 12 January 2001

DESCRIPTION :
The interface object
==============================================================================*/
{
  /* A pointer to the main Cell interface object */
  struct Cell_interface *interface;
  /* A pointer to the CMGUI interface object */
  struct Cell_cmgui_interface *cmgui_interface;
  /* A pointer to the distributed editing dialog */
  struct Distributed_editing_dialog *dialog;
  /* information about the element point being edited; note element in
     identifier is not accessed */
  struct Element_point_ranges_identifier element_point_identifier;
  int element_point_number;
  /* accessed local copy of the element being edited */
  struct FE_element *element_copy;
  FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
}; /* struct Distributed_editing_interface */

/*
Module Functions
================
*/
static int update_variable_from_element_point(
  struct Cell_variable *cell_variable,void *iterator_data_void)
/*******************************************************************************
LAST MODIFIED : 17 January 2001

DESCRIPTION :
Iterator function for updating Cell variable's from a given element point
==============================================================================*/
{
	int return_code;
  struct Iterator_data *iterator_data;
  struct Computed_field *field;
  char *value_string;
  char *name;

	ENTER(update_variable_from_element_point);
	if (cell_variable &&
    (iterator_data = (struct Iterator_data *)iterator_data_void))
	{
    name = Cell_variable_get_name(cell_variable);
    field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(name,
      Cell_cmgui_interface_get_computed_field_manager(
        iterator_data->cmgui_interface));
    if (name) DEALLOCATE(name);
    if (field)
    {
      if (value_string = Computed_field_evaluate_as_string_in_element(field,
        iterator_data->comp_no,iterator_data->element,iterator_data->xi,
        iterator_data->top_level_element))
      {
        return_code = Cell_variable_set_value_from_string(cell_variable,
          value_string);
        DEALLOCATE(value_string);
      }
      else
      {
        name = Cell_variable_get_name(cell_variable);
        display_message(WARNING_MESSAGE,"update_variable_from_element_point.  "
          "Unable to get a value for the field: %s",name);
        if (name) DEALLOCATE(name);
        return_code = 1;
      }
    }
    else
    {
      /* do nothing */
      return_code = 1;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,"update_variable_from_element_point.  "
      "Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
} /* update_variable_from_element_point() */

static int get_grid(struct Distributed_editing_interface *interface)
/*******************************************************************************
LAST MODIFIED : 16 January 2001

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

	ENTER(get_grid);
	if (interface)
	{
		return_code = 1;
		element = interface->element_point_identifier.element;
		if (element && (grid_field = FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
			Computed_field_is_scalar_integer_grid_in_element,(void *)element,
			Cell_cmgui_interface_get_computed_field_manager(
				interface->cmgui_interface))) &&
			Computed_field_get_type_finite_element(grid_field,&grid_fe_field) &&
			FE_element_field_is_grid_based(element,grid_fe_field))
		{
			get_FE_element_field_grid_map_number_in_xi(element,grid_fe_field,
				interface->element_point_identifier.number_in_xi);
			interface->element_point_identifier.xi_discretization_mode=
				XI_DISCRETIZATION_CELL_CORNERS;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_grid.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* get_grid() */

static int select_current_point(struct Distributed_editing_interface *interface)
/*******************************************************************************
LAST MODIFIED : 16 January 2001

DESCRIPTION :
Makes the currently described element point the only one in the global
selection. Does nothing if no current element point.
==============================================================================*/
{
	int return_code;
 	struct Element_point_ranges *element_point_ranges;

	ENTER(select_current_point);
	if (interface)
	{
		if (interface->element_point_identifier.element)
		{
			if (element_point_ranges = CREATE(Element_point_ranges)(
				&(interface->element_point_identifier)))
			{
				Element_point_ranges_add_range(element_point_ranges,
					interface->element_point_number,
					interface->element_point_number);
				Element_point_ranges_selection_begin_cache(
					Cell_cmgui_interface_get_element_point_ranges_selection(
            interface->cmgui_interface));
				Element_point_ranges_selection_clear(
					Cell_cmgui_interface_get_element_point_ranges_selection(
            interface->cmgui_interface));
				return_code =
					Element_point_ranges_selection_select_element_point_ranges(
            Cell_cmgui_interface_get_element_point_ranges_selection(
              interface->cmgui_interface),
						element_point_ranges);
				Element_point_ranges_selection_end_cache(
					Cell_cmgui_interface_get_element_point_ranges_selection(
            interface->cmgui_interface));
				DESTROY(Element_point_ranges)(&element_point_ranges);
			}
			else
			{
				return_code = 0;
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"select_current_point.  Failed");
			}
		}
		else
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"select_current_point.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
} /* select_current_point() */

static int calculate_xi(struct Distributed_editing_interface *interface)
/*******************************************************************************
LAST MODIFIED : 23 April 2001

DESCRIPTION :
Ensures xi is correct for the currently selected element point, if any.
==============================================================================*/
{
	int return_code;

	ENTER(calculate_xi);
	if (interface)
	{
		if (interface->element_point_identifier.element)
		{
			return_code = FE_element_get_numbered_xi_point(
				interface->element_point_identifier.element,
				interface->element_point_identifier.xi_discretization_mode,
				interface->element_point_identifier.number_in_xi,
				interface->element_point_identifier.exact_xi,
				/*coordinate_field*/(struct Computed_field *)NULL,
				/*density_field*/(struct Computed_field *)NULL,
				interface->element_point_number,
				interface->xi);
		}
		else
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"calculate_xi.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* calculate_xi() */

static int refresh_element(struct Distributed_editing_interface *interface)
/*******************************************************************************
LAST MODIFIED : 16 January 2001

DESCRIPTION :
Updates the element shown in the chooser to match that for the current point.
==============================================================================*/
{
	int return_code;
	struct FE_element *element;
 
	ENTER(refresh_element);
	if (interface)
	{
		return_code = 1;
		if (element = interface->element_point_identifier.element)
		{
			TEXT_CHOOSE_OBJECT_SET_OBJECT(FE_element)(
				interface->dialog->element_widget,element);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"refresh_element.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
} /* refresh_element() */

static int refresh_point_number_text(
  struct Distributed_editing_interface *interface)
/*******************************************************************************
LAST MODIFIED : 16 January 2001

DESCRIPTION :
Updates the point_number text field. If there is a current element point,
writes its number, otherwise N/A.
==============================================================================*/
{
	char temp_string[20];
	int return_code,is_sensitive;
 
	ENTER(refresh_point_number_text);
	if (interface)
	{
		return_code = 1;
		if (interface->element_point_identifier.element)
		{
			sprintf(temp_string,"%d",interface->element_point_number);
			XmTextFieldSetString(interface->dialog->point_number_text,temp_string);
			is_sensitive=True;
		}
		else
		{
			XmTextFieldSetString(interface->dialog->point_number_text,"N/A");
			is_sensitive=False;
		}
		XtSetSensitive(interface->dialog->point_number_text,is_sensitive);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"refresh_point_number_text.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
} /* refresh_point_number_text() */

static int refresh_grid_value_text(
  struct Distributed_editing_interface *interface)
/*******************************************************************************
LAST MODIFIED : 16 January 2001

DESCRIPTION :
Updates the grid_value text field. If there is a current element point, writes
the field value, otherwise N/A.
==============================================================================*/
{
	char *value_string;
	int is_sensitive,return_code;
	struct Computed_field *grid_field;
	struct FE_element *top_level_element;
 
	ENTER(refresh_grid_value_text);
	if (interface)
	{
		return_code = 1;
		top_level_element = (struct FE_element *)NULL;
		if ((interface->element_point_identifier.element) &&
			(grid_field=CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
				interface->dialog->grid_field_widget)))
		{
			if (value_string = Computed_field_evaluate_as_string_in_element(
				grid_field,/*component_number*/-1, 
				interface->element_point_identifier.element,
				interface->xi,top_level_element))
			{
				XmTextFieldSetString(interface->dialog->grid_value_text,
					value_string);
				DEALLOCATE(value_string);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"refresh_grid_value_text.  "
					"Could not evaluate field");
				XmTextFieldSetString(interface->dialog->grid_value_text,"ERROR");
			}
			Computed_field_clear_cache(grid_field);
			is_sensitive=True;
		}
		else
		{
			XmTextFieldSetString(interface->dialog->grid_value_text,"N/A");
			is_sensitive=False;
		}
		XtSetSensitive(interface->dialog->grid_value_text,is_sensitive);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"refresh_grid_value_text.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
} /* refresh_grid_value_text() */

static int refresh_chooser_widgets(
  struct Distributed_editing_interface *interface)
/*******************************************************************************
LAST MODIFIED : 16 January 2001

DESCRIPTION :
Fills the widgets for choosing the element point with the current values.
==============================================================================*/
{
	int return_code;
 
	ENTER(refresh_chooser_widgets);
	if (interface)
	{
		return_code = 1;
		refresh_element(interface);
		refresh_point_number_text(interface);
		refresh_grid_value_text(interface);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"refresh_chooser_widgets.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
} /* refresh_chooser_widgets */

static void update_from_element_point(
  struct Distributed_editing_interface *interface)
/*******************************************************************************
LAST MODIFIED : 16 January 2001

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
  int return_code = 0;
  struct Iterator_data iterator_data;

	ENTER(update_from_element_point);
  if (interface &&
    (element = interface->element_point_identifier.element) &&
		(top_level_element =
			interface->element_point_identifier.top_level_element) &&
		(xi = interface->xi))
  {
    comp_no = 0;
    /* get the cell_type field if possible */
    if (field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)("cell_type",
      Cell_cmgui_interface_get_computed_field_manager(
				interface->cmgui_interface)))
    {
      if (value_string = Computed_field_evaluate_as_string_in_element(
        field,comp_no,element,xi,top_level_element))
      {
        sprintf(description,"Currently editing a grid point with cell type: "
          "%s",value_string);
        DEALLOCATE(value_string);
      }
    }
    else
    {
      sprintf(description,"Can't get the cell type for this grid point");
    }
    str = XmStringCreateSimple(description);
    if (interface->dialog->description_label != (Widget)NULL)
    {
      XtVaSetValues(interface->dialog->description_label,
        XmNlabelString,str,
        NULL);
    }
    XmStringFree(str);
    /* update the variables from the element point */
    iterator_data.cmgui_interface = interface->cmgui_interface;
    iterator_data.comp_no = comp_no;
    iterator_data.element = element;
    iterator_data.xi = xi;
    iterator_data.top_level_element = top_level_element;
    return_code = FOR_EACH_OBJECT_IN_LIST(Cell_variable)(
      update_variable_from_element_point,(void *)(&iterator_data),
      Cell_interface_get_variable_list(interface->interface));
    if (!return_code)
    {
      display_message(ERROR_MESSAGE,"Cell_window_update_from_element_point. "
        "Unable to update the cell window from the element point");
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"update_from_element_point.  Invalid argument(s)");
	}
	LEAVE;
} /* update_from_element_point() */

static void element_point_ranges_selection_change(
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct Element_point_ranges_selection_changes *changes,
	void *interface_void)
/*******************************************************************************
LAST MODIFIED : 16 January 2001

DESCRIPTION :
Callback for change in the global element_point selection.

??? We want to keep track of element point selection changes, but only update
??? variable values when the distributed editing toggle is set
???
==============================================================================*/
{
	int start,stop;
	struct Element_point_ranges *element_point_ranges;
	struct Distributed_editing_interface *interface;
	struct Multi_range *ranges;

	ENTER(element_point_ranges_selection_change);
	if (element_point_ranges_selection && changes && (interface =
		(struct Distributed_editing_interface *)interface_void))
	{
    /* get the last selected element_point and put it in the cell window */
    if ((element_point_ranges =
      FIRST_OBJECT_IN_LIST_THAT(Element_point_ranges)(
        (LIST_CONDITIONAL_FUNCTION(Element_point_ranges) *)NULL,(void *)NULL,
        changes->newly_selected_element_point_ranges_list)) ||
      (element_point_ranges =
        FIND_BY_IDENTIFIER_IN_LIST(Element_point_ranges,identifier)(
          &(interface->element_point_identifier),
          Element_point_ranges_selection_get_element_point_ranges_list(
            element_point_ranges_selection))) ||
      (element_point_ranges=
        FIRST_OBJECT_IN_LIST_THAT(Element_point_ranges)(
          (LIST_CONDITIONAL_FUNCTION(Element_point_ranges) *)NULL,
          (void *)NULL,
          Element_point_ranges_selection_get_element_point_ranges_list(
            element_point_ranges_selection))))
    {
      Element_point_ranges_get_identifier(element_point_ranges,
        &(interface->element_point_identifier));
      if ((ranges = Element_point_ranges_get_ranges(element_point_ranges))&&
        Multi_range_get_range(ranges,0,&start,&stop))
      {
        interface->element_point_number = start;
      }
      else
      {
        interface->element_point_number = 0;
      }
      /* Only modify Cell variables if the distributed editing toggle is set */
      if (Distributed_editing_dialog_get_activation_state(interface->dialog))
      {
        calculate_xi(interface);
        refresh_chooser_widgets(interface);
        update_from_element_point(interface);
      }
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_point_ranges_selection_change.  "
			"Invalid argument(s)");
	}
	LEAVE;
} /* element_point_ranges_selection_change() */

static void update_grid_field(Widget widget,
	void *interface_void,void *grid_field_void)
/*******************************************************************************
LAST MODIFIED : 16 January 2001

DESCRIPTION :
Callback for change of grid field.
==============================================================================*/
{
	struct Distributed_editing_interface *interface;

	ENTER(update_grid_field);
	USE_PARAMETER(widget);
	USE_PARAMETER(grid_field_void);
	if (interface = (struct Distributed_editing_interface *)interface_void)
	{
		refresh_grid_value_text(interface);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"update_grid_field.  Invalid argument(s)");
	}
	LEAVE;
} /* update_grid_field() */

static void update_element(Widget widget,
	void *interface_void,void *element_void)
/*******************************************************************************
LAST MODIFIED : 16 January 2001

DESCRIPTION :
Callback for change of element.
==============================================================================*/
{
	struct Distributed_editing_interface *interface;

	ENTER(update_element);
	USE_PARAMETER(widget);
	if (interface = (struct Distributed_editing_interface *)interface_void)
	{
		if (interface->element_point_identifier.element =
			(struct FE_element *)element_void)
		{
			if (XI_DISCRETIZATION_CELL_CORNERS ==
				interface->element_point_identifier.xi_discretization_mode)
			{
				get_grid(interface);
			}
			calculate_xi(interface);
			select_current_point(interface);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"update_element.  Invalid argument(s)");
	}
	LEAVE;
} /* update_element() */

static int set_grid_values_from_variable(
	struct Cell_variable *variable,void *iterator_data_void)
/*******************************************************************************
LAST MODIFIED : 24 January 2001

DESCRIPTION :
Iterator function used to set the value at a given grid point from the
<variable>'s value.

Based on:
graphics/element_point_ranges.c/Field_value_index_ranges_set_grid_values()
==============================================================================*/
{
	FE_value *destination_values;
	int consistent_grid,destination_number_of_grid_values,dimension,i,j,
		*int_values,number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS],offset,
		number_of_components,return_code,source_number_of_grid_values;
	struct Computed_field *field;
	struct Element_point_ranges_set_grid_values_data *set_grid_values_data;
	struct FE_element *destination_element,*source_element;
	struct FE_field *fe_field;
  struct Iterator_data_set *iterator_data;
  int component_number;
  char *name;

	ENTER(set_grid_values_from_variable);
	if (variable && iterator_data_void &&
    (iterator_data = (struct Iterator_data_set *)iterator_data_void) &&
		(set_grid_values_data = iterator_data->set_grid_values_data) &&
		set_grid_values_data->source_identifier &&
		(source_element = set_grid_values_data->source_identifier->element) &&
		set_grid_values_data->destination_identifier &&
		(destination_element = set_grid_values_data->element_copy) &&
		set_grid_values_data->destination_element_point_numbers &&
    (name = Cell_variable_get_name(variable)))
	{
    /* Check that it is a field that we want to modify - only interested in
     * single component fields as there are no multi-component Cell variables??
     */
    if ((field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(name,
      Cell_cmgui_interface_get_computed_field_manager(
        iterator_data->interface->cmgui_interface))) &&
      (number_of_components = Computed_field_get_number_of_components(field)) &&
      (number_of_components == 1) &&
      Computed_field_is_type_finite_element(field) &&
      Computed_field_get_type_finite_element(field,&fe_field) &&
      FE_element_field_is_grid_based(destination_element,fe_field))
    {
      component_number = 0;
      /* check constistency of source element point is a grid point */
      if ((XI_DISCRETIZATION_CELL_CORNERS ==
        set_grid_values_data->source_identifier->xi_discretization_mode) &&
        Computed_field_get_native_discretization_in_element(field,
          source_element,number_in_xi))
      {
        return_code = 1;
        dimension = get_FE_element_dimension(source_element);
        /* check native discretization matches that of source_identifier,
           also calculate source number of grid point values */
        source_number_of_grid_values = 1;
        for (i=0;i<dimension;i++)
        {
          if (set_grid_values_data->source_identifier->number_in_xi[i] !=
            number_in_xi[i])
          {
            return_code = 0;
          }
          source_number_of_grid_values *= (number_in_xi[i]+1);
        }
      }
      else
      {
        return_code = 0;
      }
      if (return_code)
      {
        if ((XI_DISCRETIZATION_CELL_CORNERS ==
          set_grid_values_data->destination_identifier->xi_discretization_mode)
          && Computed_field_get_native_discretization_in_element(field,
            destination_element,number_in_xi))
        {
          consistent_grid = 1;
          dimension = get_FE_element_dimension(destination_element);
          /* check native discretization matches that of destination_identifier,
             also calculate destination number of grid point values */
          destination_number_of_grid_values = 1;
          for (i=0;i<dimension;i++)
          {
            if (set_grid_values_data->destination_identifier->number_in_xi[i] !=
              number_in_xi[i])
            {
              consistent_grid = 0;
            }
            destination_number_of_grid_values *= (number_in_xi[i]+1);
          }
        }
        else
        {
          consistent_grid = 0;
        }
        if (consistent_grid)
        {
          if (Computed_field_is_type_finite_element(field) &&
            Computed_field_get_type_finite_element(field,&fe_field) &&
            (INT_VALUE == get_FE_field_value_type(fe_field)))
          {
            /* handle integer value_type separately */
            if (return_code=get_FE_element_field_component_grid_int_values(
              destination_element,fe_field,component_number,
              &int_values))
            {
              for (j=0;j<destination_number_of_grid_values;j++)
              {
                if (Multi_range_is_value_in_range(set_grid_values_data->
                  destination_element_point_numbers,j))
                {
                  int_values[j] =
                    (int)Cell_variable_get_integer_value(variable);
                }
              }
              return_code=set_FE_element_field_component_grid_int_values(
                destination_element,fe_field,component_number,
                int_values);
              DEALLOCATE(int_values);
            }
          }
          else
          {
            if (return_code = Computed_field_get_values_in_element(field,
              destination_element,number_in_xi,&destination_values))
            {
              offset = component_number*destination_number_of_grid_values;
              for (j=0;j<destination_number_of_grid_values;j++)
              {
                if (Multi_range_is_value_in_range(set_grid_values_data->
                  destination_element_point_numbers,j))
                {
                  destination_values[offset+j] =
                    (FE_value)Cell_variable_get_real_value(variable);
                }
              }
              return_code = Computed_field_set_values_in_element(field,
                destination_element,number_in_xi,destination_values);
              DEALLOCATE(destination_values);
            }
            /* clear field cache so elements not accessed by it */
            Computed_field_clear_cache(field);
          }
        }
        else
        {
          /* clear flag to allow problem to be reported later */
          set_grid_values_data->all_points_native = 0;
        }
        if (!return_code)
        {
          display_message(ERROR_MESSAGE,
            "set_grid_values_from_variable.  Failed");
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "set_grid_values_from_variable.  "
          "Source element point is not a grid point");
      }
    }
    else
    {
      /* The field is not one Cell should modify, so do nothing */
      return_code = 1;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_grid_values_from_variable.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* set_grid_values_from_variable() */

static int cell_element_point_ranges_set_grid_values(
	struct Element_point_ranges *element_point_ranges,
	void *iterator_data_void)
/*******************************************************************************
LAST MODIFIED : 22 January 2001

DESCRIPTION :
Iterator function used to update field values at selected grid points.

Based on:
graphics/element_point_ranges.c/Element_point_ranges_set_grid_values()
==============================================================================*/
{
	int return_code;
	struct Element_point_ranges_identifier element_point_ranges_identifier;
	struct Element_point_ranges_set_grid_values_data *set_grid_values_data;
	struct FE_element *element;
  struct Iterator_data_set *iterator_data;

	ENTER(cell_element_point_ranges_set_grid_values);
	if (element_point_ranges &&
    (iterator_data = (struct Iterator_data_set *)iterator_data_void) &&
    iterator_data->interface &&
		(set_grid_values_data = iterator_data->set_grid_values_data) &&
		set_grid_values_data->source_identifier &&
    set_grid_values_data->element_manager)
	{
		/* make local element_copy from that in element_point_ranges */
		if (Element_point_ranges_get_identifier(element_point_ranges,
			&element_point_ranges_identifier) &&
			(element = element_point_ranges_identifier.element) &&
			(set_grid_values_data->element_copy =
				CREATE(FE_element)(element->identifier,element)))
		{
			/* access element_copy to be safe from ACCESS/DEACCESS cycles in
				 computed field evaluations */
			ACCESS(FE_element)(set_grid_values_data->element_copy);
			/* pass element_point_ranges to compare discretizations */
			set_grid_values_data->destination_identifier =
				&element_point_ranges_identifier;
			set_grid_values_data->destination_element_point_numbers =
				Element_point_ranges_get_ranges(element_point_ranges);
			/* set values in the local element_copy */
      if (return_code = FOR_EACH_OBJECT_IN_LIST(Cell_variable)(
        set_grid_values_from_variable,iterator_data_void,
        Cell_interface_get_variable_list(iterator_data->interface->interface)))
			{
				return_code = MANAGER_MODIFY_NOT_IDENTIFIER(FE_element,identifier)(
					element,set_grid_values_data->element_copy,
					set_grid_values_data->element_manager);
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"cell_element_point_ranges_set_grid_values.  "
          "Could not set values");
			}
			DEACCESS(FE_element)(&(set_grid_values_data->element_copy));
		}
		else
		{
			display_message(ERROR_MESSAGE,
        "cell_element_point_ranges_set_grid_values.  "
				"Could not make local copy of element");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"cell_element_point_ranges_set_grid_values.  "
      "Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	return(return_code);
} /* cell_element_point_ranges_set_grid_values() */

/*
Global Functions
================
*/
struct Distributed_editing_interface *CREATE(Distributed_editing_interface)(
  struct Cell_interface *cell_interface,
  struct Cell_cmgui_interface *cmgui_interface)
/*******************************************************************************
LAST MODIFIED : 12 January 2001

DESCRIPTION :
Creates a distributed editing interface object.
==============================================================================*/
{
  struct Distributed_editing_interface *distributed_editing_interface;
	struct Element_point_ranges *element_point_ranges;
	struct Multi_range *ranges;
  int i,start,stop,number_of_faces;

  ENTER(CREATE(Distributed_editing_interface));
  if (cell_interface && cmgui_interface)
  {
    /* Allocate memory for the object */
    if (ALLOCATE(distributed_editing_interface,
      struct Distributed_editing_interface,1))
    {
      /* Initialise the object */
      distributed_editing_interface->interface = cell_interface;
      distributed_editing_interface->cmgui_interface = cmgui_interface;
      distributed_editing_interface->dialog =
        (struct Distributed_editing_dialog *)NULL;
      distributed_editing_interface->element_point_identifier.element =
        (struct FE_element *)NULL;
      distributed_editing_interface->element_point_identifier.top_level_element=
        (struct FE_element *)NULL;
      distributed_editing_interface->element_point_identifier.xi_discretization_mode =
        XI_DISCRETIZATION_EXACT_XI;
      distributed_editing_interface->element_copy =
        (struct FE_element *)NULL;
      for (i=0;i<MAXIMUM_ELEMENT_XI_DIMENSIONS;i++)
      {
        distributed_editing_interface->element_point_identifier.number_in_xi[i]
          = 1;
        distributed_editing_interface->xi[i] =
          distributed_editing_interface->element_point_identifier.exact_xi[i]
          = 0.5;
      }
      distributed_editing_interface->element_point_number = 0;

      /* Distributed editing - From element/element_point_viewer.c
       *
       * Create the widgets/structures for selecting element (grid)
       * points.
       */
      if (element_point_ranges =
        FIRST_OBJECT_IN_LIST_THAT(Element_point_ranges)
        ((LIST_CONDITIONAL_FUNCTION(Element_point_ranges) *)NULL,
          (void *)NULL,
          Element_point_ranges_selection_get_element_point_ranges_list(
            Cell_cmgui_interface_get_element_point_ranges_selection(
              distributed_editing_interface->cmgui_interface))))
      {
        Element_point_ranges_get_identifier(element_point_ranges,
          &(distributed_editing_interface->element_point_identifier));
        if ((ranges = Element_point_ranges_get_ranges(
          element_point_ranges)) && Multi_range_get_range(ranges,0,
            &start,&stop))
        {
          distributed_editing_interface->element_point_number = start;
        }
      }
      else
      {
        /* try to get any point in the first top-level element
         * we can find.
         */
        if (distributed_editing_interface->element_point_identifier.element =
          FIRST_OBJECT_IN_MANAGER_THAT(FE_element)(
            FE_element_is_top_level,(void *)NULL,
            Cell_cmgui_interface_get_element_manager(
              distributed_editing_interface->cmgui_interface)))
        {
          distributed_editing_interface->element_point_identifier.top_level_element
            = distributed_editing_interface->element_point_identifier.element;
        }
        else
        {
          distributed_editing_interface->element_point_identifier.top_level_element
            = (struct FE_element *)NULL;
        }
        /* try to get a grid point, if possible */
        get_grid(distributed_editing_interface);
        /* make the element point the only one in the global
         * selection
         */
        select_current_point(distributed_editing_interface);
      }
      calculate_xi(distributed_editing_interface);
      if (distributed_editing_interface->element_point_identifier.top_level_element)
      {
        if (distributed_editing_interface->element_copy = ACCESS(FE_element)(
          CREATE(FE_element)(
            distributed_editing_interface->element_point_identifier.top_level_element->identifier,
            distributed_editing_interface->element_point_identifier.top_level_element)))
        {
          /* clear the faces of element_copy as messes up exterior
             calculations for graphics created from them */
          number_of_faces =
            distributed_editing_interface->element_copy->shape->number_of_faces;
          for (i=0;i<number_of_faces;i++)
          {
            set_FE_element_face(distributed_editing_interface->element_copy,i,
              (struct FE_element *)NULL);
          }
        }
      }
      else
      {
        distributed_editing_interface->element_copy=(struct FE_element *)NULL;
      }
      /* get callbacks from global element_point selection */
      Element_point_ranges_selection_add_callback(
        Cell_cmgui_interface_get_element_point_ranges_selection(
          distributed_editing_interface->cmgui_interface),
        element_point_ranges_selection_change,
        (void *)distributed_editing_interface);
    }
    else
    {
      display_message(ERROR_MESSAGE,"CREATE(Distributed_editing_interface).  "
        "Unable to allocate memory for the object");
      distributed_editing_interface =
        (struct Distributed_editing_interface *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Distributed_editing_interface).  "
      "Invalid argument(s)");
    distributed_editing_interface =
      (struct Distributed_editing_interface *)NULL;
  }
  LEAVE;
  return(distributed_editing_interface);
} /* CREATE(Distributed_editing_interface)() */

int DESTROY(Distributed_editing_interface)(
  struct Distributed_editing_interface **interface_address)
/*******************************************************************************
LAST MODIFIED : 12 January 2001

DESCRIPTION :
Destroys a distributed editing interface object.
==============================================================================*/
{
  int return_code = 0;
  struct Distributed_editing_interface *interface;

  ENTER(DESTROY(Distributed_editing_interface));
  if (interface_address && (interface = *interface_address))
  {
    if (interface->dialog)
    {
      DESTROY(Distributed_editing_dialog)(&(interface->dialog));
    }
    DEALLOCATE(*interface_address);
    *interface_address = (struct Distributed_editing_interface *)NULL;
    return_code = 1;
  }
  LEAVE;
  return(return_code);
} /* DESTROY(Distributed_editing_interface)() */

int Distributed_editing_interface_pop_up_dialog(
  struct Distributed_editing_interface *interface,Widget parent,
  struct User_interface *user_interface,Widget activation)
/*******************************************************************************
LAST MODIFIED : 15 January 2001

DESCRIPTION :
Pops up the distributed editing dialog, creating it if nescessary. <activation>
is the toggle button which activated the dialog.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Distributed_editing_interface_pop_up_dialog);
  if (interface)
  {
    if (interface->dialog == (struct Distributed_editing_dialog *)NULL)
    {
      interface->dialog = CREATE(Distributed_editing_dialog)(interface,parent,
        user_interface,activation);
    }
    if (interface->dialog != (struct Distributed_editing_dialog *)NULL)
    {
      return_code = Distributed_editing_dialog_pop_up(interface->dialog);
      /* Update the Cell interface from the current grid point ?? */
      if (interface->element_point_number >= 0)
      {
        calculate_xi(interface);
        refresh_chooser_widgets(interface);
        update_from_element_point(interface);
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "Distributed_editing_interface_pop_up_dialog.  "
        "Missing dialog");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Distributed_editing_interface_pop_up_dialog.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Distributed_editing_interface_pop_up_dialog() */

int Distributed_editing_interface_pop_down_dialog(
  struct Distributed_editing_interface *interface)
/*******************************************************************************
LAST MODIFIED : 22 January 2001

DESCRIPTION :
Pops down the distributed editing dialog.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Distributed_editing_interface_pop_down_dialog);
  if (interface)
  {
    if (interface->dialog != (struct Distributed_editing_dialog *)NULL)
    {
      return_code = Distributed_editing_dialog_pop_down(interface->dialog);
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "Distributed_editing_interface_pop_down_dialog.  "
        "Missing dialog");
      return_code = 0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Distributed_editing_interface_pop_down_dialog.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Distributed_editing_interface_pop_down_dialog() */

void Distributed_editing_interface_point_number_text_CB(Widget widget,
	void *interface_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 16 January 2001

DESCRIPTION :
Called when entry is made into the point_number_text field.
==============================================================================*/
{
	char *value_string;
	int element_point_number;
	struct Distributed_editing_interface *interface;
	XmAnyCallbackStruct *any_callback;

	ENTER(Distributed_editing_interface_point_number_text_CB);
	USE_PARAMETER(widget);
	if ((interface = (struct Distributed_editing_interface *)interface_void) &&
		(any_callback = (XmAnyCallbackStruct *)call_data))
	{
		if (XmCR_ACTIVATE == any_callback->reason)
		{
			/* Get the text string */
			if (value_string =
				XmTextFieldGetString(interface->dialog->point_number_text))
			{
				if ((1 == sscanf(value_string,"%d",&element_point_number)) &&
					Element_point_ranges_identifier_element_point_number_is_valid(
						&(interface->element_point_identifier),
						element_point_number))
				{
					interface->element_point_number = element_point_number;
					select_current_point(interface);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Distributed_editing_interface_point_number_text_CB.  "
            "Invalid point number");
				}
				XtFree(value_string);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Distributed_editing_interface_point_number_text_CB.  Missing text");
			}
		}
		/* always restore point_number_text to actual value stored */
		refresh_point_number_text(interface);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Distributed_editing_interface_point_number_text_CB.  "
      "Invalid argument(s)");
	}
	LEAVE;
} /* Distributed_editing_interface_point_number_text_CB() */

void Distributed_editing_interface_grid_value_text_CB(Widget widget,
	void *interface_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 16 January 2001

DESCRIPTION :
Called when entry is made into the grid_value_text field.
==============================================================================*/
{
	char *value_string;
	int grid_value;
	struct Computed_field *grid_field;
	struct Distributed_editing_interface *interface;
	struct FE_element_grid_to_Element_point_ranges_list_data grid_to_list_data;
	struct FE_field *grid_fe_field;
	XmAnyCallbackStruct *any_callback;

	ENTER(Distributed_editing_interface_grid_value_text_CB);
	USE_PARAMETER(widget);
	if ((interface = (struct Distributed_editing_interface *)interface_void) &&
		(any_callback = (XmAnyCallbackStruct *)call_data))
	{
		if (XmCR_ACTIVATE == any_callback->reason)
		{
			/* Get the text string */
			if (value_string =
				XmTextFieldGetString(interface->dialog->grid_value_text))
			{
				if ((grid_field = CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
					interface->dialog->grid_field_widget)) &&
					Computed_field_get_type_finite_element(grid_field,&grid_fe_field))
				{
					if (1 == sscanf(value_string,"%d",&grid_value))
					{
						if ((grid_to_list_data.grid_value_ranges = CREATE(Multi_range)()) &&
							Multi_range_add_range(grid_to_list_data.grid_value_ranges,
								grid_value,grid_value))
						{
							if (grid_to_list_data.element_point_ranges_list =
								CREATE(LIST(Element_point_ranges))())
							{
								grid_to_list_data.grid_fe_field=grid_fe_field;
								/* inefficient: go through every element in manager */
								FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
									FE_element_grid_to_Element_point_ranges_list,
									(void *)&grid_to_list_data,
									Cell_cmgui_interface_get_element_manager(
                    interface->cmgui_interface));
								if (0 < NUMBER_IN_LIST(Element_point_ranges)(
									grid_to_list_data.element_point_ranges_list))
								{
									Element_point_ranges_selection_begin_cache(
										Cell_cmgui_interface_get_element_point_ranges_selection(
                      interface->cmgui_interface));
									Element_point_ranges_selection_clear(
										Cell_cmgui_interface_get_element_point_ranges_selection(
                      interface->cmgui_interface));
									FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
										Element_point_ranges_select,
										(void *)
                    Cell_cmgui_interface_get_element_point_ranges_selection(
                      interface->cmgui_interface),
										grid_to_list_data.element_point_ranges_list);
									Element_point_ranges_selection_end_cache(
										Cell_cmgui_interface_get_element_point_ranges_selection(
                      interface->cmgui_interface));
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
					"Distributed_editing_interface_grid_value_text_CB.  Missing text");
			}
		}
		/* always restore grid_value_text to actual value stored */
		refresh_grid_value_text(interface);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Distributed_editing_interface_grid_value_text_CB.  "
      "Invalid argument(s)");
	}
	LEAVE;
} /* Distributed_editing_interface_grid_value_text_CB() */

int Distributed_editing_interface_create_choosers(
  struct Distributed_editing_interface *interface,
  void *dialog_void)
/*******************************************************************************
LAST MODIFIED : 17 January 2001

DESCRIPTION :
Creates the chooser widgets for the distributed editing dialog.
==============================================================================*/
{
  int return_code = 0;
  struct Distributed_editing_dialog *dialog;
  struct MANAGER(Computed_field) *computed_field_manager;
  struct Computed_field *grid_field;
	struct Callback_data callback;
  
  ENTER(Distributed_editing_interface_create_choosers);
  if (interface && (dialog = (struct Distributed_editing_dialog *)dialog_void))
  {
    return_code = 1;
    /* create the element chooser */
    if (!(dialog->element_widget =
      CREATE_TEXT_CHOOSE_OBJECT_WIDGET(FE_element)(
        dialog->element_form,
        interface->element_point_identifier.element,
        Cell_cmgui_interface_get_element_manager(
          interface->cmgui_interface),
        (MANAGER_CONDITIONAL_FUNCTION(FE_element) *)NULL,
        (void *)NULL,
        FE_element_to_any_element_string,
        any_element_string_to_FE_element)))
    {
      return_code = 0;
    }
    /* set the initial grid field to use for selecting
     * element points - if a grid_point_number field exists
     * use that, otherwise just take the first one which is
     * suitable
     */
    computed_field_manager = Cell_cmgui_interface_get_computed_field_manager(
      interface->cmgui_interface);
    if (!(grid_field =
      FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
        "grid_point_number",computed_field_manager)))
    {
      grid_field = FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
        Computed_field_is_scalar_integer,(void *)NULL,
        computed_field_manager);
    }
    /* create the grid field chooser for setting the field
     * used to select element points
     */
    if (!(dialog->grid_field_widget =
      CREATE_CHOOSE_OBJECT_WIDGET(Computed_field)(
        dialog->grid_field_form,grid_field,
        computed_field_manager,Computed_field_is_scalar_integer,
        (void *)interface->element_copy)))
    {
      return_code = 0;
    }
    if (return_code)
    {
      /* set the callback for changing the grid field */
      callback.data = (void *)interface;
      callback.procedure = update_grid_field;
      CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
        dialog->grid_field_widget,&callback);
      /* set the callback for changing the element */
      callback.procedure = update_element;
      TEXT_CHOOSE_OBJECT_SET_CALLBACK(FE_element)(
        dialog->element_widget,&callback);
      /* update the widgets ?? */
      /* At this time, the dialog field of the interface won't have been set,
       * so set it here */
      interface->dialog = dialog;
      refresh_chooser_widgets(interface);
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "Distributed_editing_interface_create_choosers.  "
        "Unable to intialise the widgets");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Distributed_editing_interface_create_choosers.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Distributed_editing_interface_create_choosers() */

int Distributed_editing_interface_update_from_element_point(
  struct Distributed_editing_interface *interface)
/*******************************************************************************
LAST MODIFIED : 23 January 2001

DESCRIPTION :
Called from the "reset" button callback in the distributed editing dialog -
simply resets the variable values to the field values at the current element
point.
==============================================================================*/
{
  int return_code = 0;

  ENTER(Distributed_editing_interface_update_from_element_point);
  if (interface)
  {
    update_from_element_point(interface);
    return_code = 1;
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Distributed_editing_interface_update_from_element_point.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Distributed_editing_interfacae_update_from_element_point() */

int Distributed_editing_interface_update_element_point(
  struct Distributed_editing_interface *interface,int apply_all)
/*******************************************************************************
LAST MODIFIED : 23 January 2001

DESCRIPTION :
Called from the "apply" and "apply all" button callbacks in the distributed
editing dialog. If <apply_all> is true, then all selected points will have
their field values updated (where possible), otherwise only alters the current
element point.

SEE-ALSO : element/element_point_viewer.c/Element_point_viewer_apply_changes()
==============================================================================*/
{
  int return_code = 0;
  struct LIST(Element_point_ranges) *element_point_ranges_list;
  struct Computed_field *match_grid_field;
	struct Element_point_ranges_grid_to_multi_range_data grid_to_multi_range_data;
  char *field_name;
	struct Element_point_ranges_identifier source_identifier;
	struct Element_point_ranges_set_grid_values_data set_grid_values_data;
	struct FE_element_grid_to_Element_point_ranges_list_data grid_to_list_data;
  struct Iterator_data_set iterator_data;

  ENTER(Distributed_editing_interface_update_element_point);
  if (interface)
  {
    if (element_point_ranges_list = CREATE(LIST(Element_point_ranges))())
    {
      /* put current element point in list */
      return_code = Element_point_ranges_list_add_element_point(
        element_point_ranges_list,&(interface->element_point_identifier),
        interface->element_point_number);
      /* if apply_all, add all other selected element points */
      if (return_code && apply_all)
      {
        FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
          Element_point_ranges_add_to_list,(void *)element_point_ranges_list,
          Element_point_ranges_selection_get_element_point_ranges_list(
            Cell_cmgui_interface_get_element_point_ranges_selection(
              interface->cmgui_interface)));
      }
      /* if match_grid_field, get range of its values for all points in
         element_point_ranges_list, then get list of all grid-points with
         those values. Usually this is the grid_point_number field, and this
         feature is used to overcome the fact that we store values for same-
         numbered grid-point more than once on common element boundaries */
      /* For now, assume that it is the grid_point_number field and get it
         here */
      match_grid_field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
        "grid_point_number",Cell_cmgui_interface_get_computed_field_manager(
          interface->cmgui_interface));
      if (return_code && match_grid_field)
      {
        /* check field is wrapper for single component integer FE_field */
        if (Computed_field_get_type_finite_element(
          match_grid_field,&(grid_to_multi_range_data.grid_fe_field)) &&
          (get_FE_field_number_of_components(
            grid_to_multi_range_data.grid_fe_field) == 1) &&
          (INT_VALUE == get_FE_field_value_type(
            grid_to_multi_range_data.grid_fe_field)))
        {
          /* get multi-range of values of match_grid_field for points in
             element_point_ranges_list */
          if (grid_to_multi_range_data.multi_range = CREATE(Multi_range)())
          {
            /* if following flag is cleared it means that some of the selected
               element points are not grid points */
            grid_to_multi_range_data.all_points_native = 1;
            return_code = FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
              Element_point_ranges_grid_to_multi_range,
              (void *)&grid_to_multi_range_data,element_point_ranges_list);
            if (return_code)
            {
              /* add all points with match_grid_field value in multi-range
                 to those already in element_point_ranges_list */
              grid_to_list_data.element_point_ranges_list=
                element_point_ranges_list;
              grid_to_list_data.grid_fe_field=
                grid_to_multi_range_data.grid_fe_field;
              grid_to_list_data.grid_value_ranges=
                grid_to_multi_range_data.multi_range;
              /* inefficient: go through every element in manager */
              return_code = FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
                FE_element_grid_to_Element_point_ranges_list,
                (void *)&grid_to_list_data,
                Cell_cmgui_interface_get_element_manager(
                  interface->cmgui_interface));
            }
            DESTROY(Multi_range)(&(grid_to_multi_range_data.multi_range));
          }
          else
          {
            return_code = 0;
          }
          if (!return_code)
          {
            GET_NAME(Computed_field)(match_grid_field,&field_name);
            display_message(WARNING_MESSAGE,
              "Distributed_editing_interface_update_element_point.  "
              "Could not set same values for points with same value "
              "of field %s",field_name);
            DEALLOCATE(field_name);
          }
        }
        else
        {
          display_message(WARNING_MESSAGE,
            "Distributed_editing_interface_update_element_point.  "
            "Invalid match_grid_field");
          return_code = 0;
        }
      }
      if (return_code)
      {
        if (0 < NUMBER_IN_LIST(Element_point_ranges)(element_point_ranges_list))
        {
          /* to modify, need the element point to take values from - here we
           * hijack the Element_point_ranges_set_grid_values_data structure
           * from graphics/element_point_ranges.h to pass information through
           * to Cell's own element point updating routines. */
          if (return_code = COPY(Element_point_ranges_identifier)(
            &source_identifier,&(interface->element_point_identifier)))
          {
            source_identifier.element = interface->element_copy;
            /* note values taken from the local element_copy... */
            set_grid_values_data.source_identifier = &source_identifier;
            set_grid_values_data.source_element_point_number =
              interface->element_point_number;
            /* need the components that have been modified ... not for cell */
            set_grid_values_data.field_component_ranges_list =
              (struct LIST(Field_value_index_ranges) *)NULL;
            /* ... and the manager to modify them in */
            set_grid_values_data.element_manager =
              Cell_cmgui_interface_get_element_manager(
                interface->cmgui_interface);
            /* if following flag is cleared it means that some of the selected
               element points are not grid points */
            set_grid_values_data.all_points_native = 1;
            /* Also need the distributed editing interface object!! */
            iterator_data.interface = interface;
            iterator_data.set_grid_values_data = &set_grid_values_data;
            /* cache manager modifies when more than one being changed */
            MANAGER_BEGIN_CACHE(FE_element)(
              Cell_cmgui_interface_get_element_manager(
                interface->cmgui_interface));
            return_code = FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
              cell_element_point_ranges_set_grid_values,
              (void *)&iterator_data,element_point_ranges_list);
            if (!set_grid_values_data.all_points_native)
            {
              display_message(WARNING_MESSAGE,
                "Values only set at element points on grid");
            }
            MANAGER_END_CACHE(FE_element)(
              Cell_cmgui_interface_get_element_manager(
                interface->cmgui_interface));
          }
          if (!return_code)
          {
            display_message(ERROR_MESSAGE,
              "Distributed_editing_interface_update_element_point.  "
              "Could not set values");
          }
        }
        else
        {
          display_message(WARNING_MESSAGE,
            "Element_point_viewer_apply_changes.  "
            "No grid points to apply changes to");
          return_code=0;
        }
      }
      DESTROY(LIST(Element_point_ranges))(&(element_point_ranges_list));
    }
    else
    {
      display_message(WARNING_MESSAGE,"Cell_window_to_element_point. "
        "Could not make list");
      return_code=0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Distributed_editing_interface_update_element_point.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Distributed_editing_interfacae_update_element_point() */

int Distributed_editing_interface_has_element_copy(
  struct Distributed_editing_interface *interface)
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
Returns 1 if the <interface> contains a valid element_copy, otherwise returns 0
==============================================================================*/
{
  int return_code = 0;

  ENTER(Distributed_editing_interface_has_element_copy);
  if (interface)
  {
    if (interface->element_copy)
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
      "Distributed_editing_interface_has_element_copy.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Distributed_editing_interface_has_element_copy() */

struct FE_element *Distributed_editing_interface_get_element_copy(
  struct Distributed_editing_interface *interface)
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
Returns the element_copy for the given <interface>
==============================================================================*/
{
  struct FE_element *element_copy;

  ENTER(Distributed_editing_interface_has_element_copy);
  if (interface)
  {
    element_copy = interface->element_copy;
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Distributed_editing_interface_has_element_copy.  "
      "Invalid argument(s)");
    element_copy = (struct FE_element *)NULL;
  }
  LEAVE;
  return(element_copy);
} /* Distributed_editing_interface_get_element_copy() */


#endif /* defined (CELL_DISTRIBUTED) */
