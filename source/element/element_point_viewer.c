/*******************************************************************************
FILE : element_point_viewer.c

LAST MODIFIED : 19 March 2001

DESCRIPTION :
Dialog for selecting an element point, viewing and editing its fields and
applying changes. Works with Element_point_ranges_selection to display the last
selected element point, or set it if entered in this dialog.
==============================================================================*/
#if defined (MOTIF)
#include <Xm/Protocols.h>
#include <Xm/MwmUtil.h>
#include <Xm/Xm.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
#endif /* defined (MOTIF) */
#include "choose/choose_enumerator.h"
#include "choose/text_choose_fe_element.h"
#include "choose/choose_computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "element/element_point_viewer_widget.h"
#include "element/element_point_viewer.h"
#include "element/element_point_viewer.uidh"
#include "finite_element/field_value_index_ranges.h"
#include "general/debug.h"
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/

#if defined (MOTIF)
static int element_point_viewer_hierarchy_open=0;
static MrmHierarchy element_point_viewer_hierarchy;
#endif /* defined (MOTIF) */

struct Element_point_viewer
/*******************************************************************************
LAST MODIFIED : 30 June 2000

DESCRIPTION :
Contains all the information carried by the element_point_viewer widget.
==============================================================================*/
{
	/* global data */
	struct Computed_field_package *computed_field_package;
	struct Element_point_viewer **element_point_viewer_address;
	struct MANAGER(FE_element) *element_manager;
	void *element_manager_callback_id;
	struct MANAGER(FE_node) *node_manager;
	void *node_manager_callback_id;
	struct Element_point_ranges_selection *element_point_ranges_selection;
	struct User_interface *user_interface;
	/* information about the element point being edited; note element in
		 identifier is not accessed */
	struct Element_point_ranges_identifier element_point_identifier;
	int element_point_number;
	/* accessed local copy of the element being edited */
	struct FE_element *element_copy;
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	/* field components whose values have been modified stored in following */
	struct LIST(Field_value_index_ranges) *modified_field_components;
	/* apply will set same values to all grid points with this field the same,
		 if non-NULL */
	struct Computed_field *match_grid_field;
	/* widgets */
	Pixel label_background_color,label_foreground_color,
		textfield_background_color,textfield_bottom_shadow_color,
		textfield_foreground_color,textfield_top_shadow_color;
	Widget element_form,element_widget,
		top_level_element_form,top_level_element_widget,
		xi_discretization_mode_form,xi_discretization_mode_widget,
		discretization_number_entry,discretization_text,point_number_text,xi_text,
		grid_number_entry,grid_field_form,grid_field_widget,grid_value_text,
		viewer_form,viewer_widget,apply_options_button,apply_options,
		match_grid_button,match_grid_field_form,match_grid_field_widget;
	Widget widget,window_shell;
}; /* element_point_viewer_struct */

/*
Module functions
----------------
*/

DECLARE_DIALOG_IDENTIFY_FUNCTION(element_point_viewer,Element_point_viewer, \
	element_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(element_point_viewer,Element_point_viewer, \
	top_level_element_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(element_point_viewer,Element_point_viewer, \
	xi_discretization_mode_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(element_point_viewer,Element_point_viewer, \
	discretization_number_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(element_point_viewer,Element_point_viewer, \
	discretization_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(element_point_viewer,Element_point_viewer, \
	point_number_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(element_point_viewer,Element_point_viewer, \
	xi_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(element_point_viewer,Element_point_viewer, \
	grid_number_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(element_point_viewer,Element_point_viewer, \
	grid_field_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(element_point_viewer,Element_point_viewer, \
	grid_value_text)
DECLARE_DIALOG_IDENTIFY_FUNCTION(element_point_viewer,Element_point_viewer, \
	viewer_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(element_point_viewer,Element_point_viewer, \
	apply_options_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(element_point_viewer,Element_point_viewer, \
	apply_options)
DECLARE_DIALOG_IDENTIFY_FUNCTION(element_point_viewer,Element_point_viewer, \
	match_grid_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(element_point_viewer,Element_point_viewer, \
	match_grid_field_form)

static int Element_point_viewer_get_grid(
	struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 30 May 2000

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

	ENTER(Element_point_viewer_get_grid);
	if (element_point_viewer)
	{
		return_code=1;
		element=element_point_viewer->element_point_identifier.element;
		if (element&&(grid_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
			Computed_field_is_scalar_integer_grid_in_element,(void *)element,
			Computed_field_package_get_computed_field_manager(
				element_point_viewer->computed_field_package)))&&
			Computed_field_get_type_finite_element(grid_field,&grid_fe_field)&&
			FE_element_field_is_grid_based(element,grid_fe_field))
		{
			get_FE_element_field_grid_map_number_in_xi(element,grid_fe_field,
				element_point_viewer->element_point_identifier.number_in_xi);
			element_point_viewer->element_point_identifier.xi_discretization_mode=
				XI_DISCRETIZATION_CELL_CORNERS;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_get_grid.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_viewer_get_grid */

static int Element_point_viewer_calculate_xi(
	struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 7 June 2000

DESCRIPTION :
Ensures xi is correct for the currently selected element point, if any.
==============================================================================*/
{
	int return_code;
	struct FE_element *element;

	ENTER(Element_point_viewer_calculate_xi);
	if (element_point_viewer)
	{
		if (element=element_point_viewer->element_point_identifier.element)
		{
			return_code=Xi_discretization_mode_get_element_point_xi(
				element_point_viewer->element_point_identifier.xi_discretization_mode,
				get_FE_element_dimension(element),
				element_point_viewer->element_point_identifier.number_in_xi,
				element_point_viewer->element_point_identifier.exact_xi,
				element_point_viewer->element_point_number,
				element_point_viewer->xi);
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_calculate_xi.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_viewer_calculate_xi */

static int Element_point_viewer_set_viewer_element_point(
	struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 20 June 2000

DESCRIPTION :
Gets the current element_point, makes a copy of its element if not NULL,
and passes it to the element_point_viewer_widget.
==============================================================================*/
{
	int i,number_of_faces,temp_element_point_number,return_code;
	struct Element_point_ranges_identifier temp_element_point_identifier;

	ENTER(Element_point_viewer_set_viewer_element_point);
	if (element_point_viewer)
	{
		REACCESS(FE_element)(&(element_point_viewer->element_copy),
			(struct FE_element *)NULL);
		temp_element_point_number=element_point_viewer->element_point_number;
		COPY(Element_point_ranges_identifier)(&temp_element_point_identifier,
			&(element_point_viewer->element_point_identifier));
		if (temp_element_point_identifier.element)
		{
			if (Element_point_ranges_identifier_is_valid(
				&temp_element_point_identifier))
			{
				Element_point_make_top_level(&temp_element_point_identifier,
					&temp_element_point_number);
			}
			/* copy the element - now guaranteed to be top-level */
			if (element_point_viewer->element_copy=ACCESS(FE_element)(
				CREATE(FE_element)(temp_element_point_identifier.element->identifier,
					temp_element_point_identifier.element)))
			{
				/* clear the faces of element_copy as messes up exterior calculations
					 for graphics created from them */
				number_of_faces=
					element_point_viewer->element_copy->shape->number_of_faces;
				for (i=0;i<number_of_faces;i++)
				{
					set_FE_element_face(element_point_viewer->element_copy,i,
						(struct FE_element *)NULL);
				}
			}
		}
		/* pass identifier with copy_element to viewer widget */
		temp_element_point_identifier.element=element_point_viewer->element_copy;
		temp_element_point_identifier.top_level_element=
			element_point_viewer->element_copy;
		/* clear modified_components */
		REMOVE_ALL_OBJECTS_FROM_LIST(Field_value_index_ranges)(
			element_point_viewer->modified_field_components);
		element_point_viewer_widget_set_element_point(
			element_point_viewer->viewer_widget,
			&temp_element_point_identifier,temp_element_point_number);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_set_viewer_element_point.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_viewer_set_viewer_element_point */

static int Element_point_viewer_select_current_point(
	struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 30 May 2000

DESCRIPTION :
Makes the currently described element point the only one in the global
selection. Does nothing if no current element point.
==============================================================================*/
{
	int return_code;
 	struct Element_point_ranges *element_point_ranges;

	ENTER(Element_point_viewer_select_current_point);
	if (element_point_viewer)
	{
		if (element_point_viewer->element_point_identifier.element)
		{
			if (element_point_ranges=CREATE(Element_point_ranges)(
				&(element_point_viewer->element_point_identifier)))
			{
				Element_point_ranges_add_range(element_point_ranges,
					element_point_viewer->element_point_number,
					element_point_viewer->element_point_number);
				Element_point_ranges_selection_begin_cache(
					element_point_viewer->element_point_ranges_selection);
				Element_point_ranges_selection_clear(
					element_point_viewer->element_point_ranges_selection);
				return_code=
					Element_point_ranges_selection_select_element_point_ranges(
						element_point_viewer->element_point_ranges_selection,
						element_point_ranges);
				Element_point_ranges_selection_end_cache(
					element_point_viewer->element_point_ranges_selection);
				DESTROY(Element_point_ranges)(&element_point_ranges);
			}
			else
			{
				return_code=0;
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"Element_point_viewer_select_current_point.  Failed");
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
			"Element_point_viewer_select_current_point.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_viewer_select_current_point */

static int Element_point_viewer_refresh_elements(
	struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 20 June 2000

DESCRIPTION :
Updates the element shown in the chooser to match that for the current point.
==============================================================================*/
{
	int return_code;
 
	ENTER(Element_point_viewer_refresh_elements);
	if (element_point_viewer)
	{
		return_code=1;
		TEXT_CHOOSE_OBJECT_SET_OBJECT(FE_element)(
			element_point_viewer->element_widget,
			element_point_viewer->element_point_identifier.element);
		TEXT_CHOOSE_OBJECT_SET_OBJECT(FE_element)(
			element_point_viewer->top_level_element_widget,
			element_point_viewer->element_point_identifier.top_level_element);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_refresh_elements.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_viewer_refresh_elements */

static int Element_point_viewer_refresh_xi_discretization_mode(
	struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 19 March 2001

DESCRIPTION :
Updates the Xi_discretization_mode shown in the chooser to match that for the
current point.
==============================================================================*/
{
	int is_sensitive,return_code;
 
	ENTER(Element_point_viewer_refresh_xi_discretization_mode);
	if (element_point_viewer)
	{
		return_code=1;
		if (element_point_viewer->element_point_identifier.element)
		{
			choose_enumerator_set_string(
				element_point_viewer->xi_discretization_mode_widget,
				ENUMERATOR_STRING(Xi_discretization_mode)(element_point_viewer->
					element_point_identifier.xi_discretization_mode));
			is_sensitive=True;
		}
		else
		{
			is_sensitive=False;
		}
		XtSetSensitive(element_point_viewer->xi_discretization_mode_widget,
			is_sensitive);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_refresh_xi_discretization_mode.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_viewer_refresh_xi_discretization_mode */

static int Element_point_viewer_set_textfield_edit_status(
	struct Element_point_viewer *element_point_viewer,Widget textfield_widget,
	int is_editable)
/*******************************************************************************
LAST MODIFIED : 30 June 2000

DESCRIPTION :
Sets colours and XmNeditable flag of <textfield_widget> to match <is_editable>.
==============================================================================*/
{
	int return_code;

	ENTER(Element_point_viewer_set_textfield_edit_status);
	if (element_point_viewer&&textfield_widget)
	{
		if (is_editable)
		{
			/* make it look like a textfield */
			XtVaSetValues(textfield_widget,XmNforeground,
				element_point_viewer->textfield_foreground_color,NULL);
			XtVaSetValues(textfield_widget,XmNbackground,
				element_point_viewer->textfield_background_color,NULL);
			XtVaSetValues(textfield_widget,XmNbottomShadowColor,
				element_point_viewer->textfield_bottom_shadow_color,NULL);
			XtVaSetValues(textfield_widget,XmNtopShadowColor,
				element_point_viewer->textfield_top_shadow_color,NULL);
			XtVaSetValues(textfield_widget,XmNeditable,True,NULL);
		}
		else
		{
			/* make it look like a label */
			XtVaSetValues(textfield_widget,XmNforeground,
				element_point_viewer->label_foreground_color,NULL);
			XtVaSetValues(textfield_widget,XmNbackground,
				element_point_viewer->label_background_color,NULL);
			XtVaSetValues(textfield_widget,XmNbottomShadowColor,
				element_point_viewer->textfield_bottom_shadow_color,NULL);
			XtVaSetValues(textfield_widget,XmNtopShadowColor,
				element_point_viewer->textfield_bottom_shadow_color,NULL);
			XtVaSetValues(textfield_widget,XmNeditable,False,NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_set_textfield_edit_status.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_viewer_set_textfield_edit_status */

static int Element_point_viewer_refresh_discretization_text(
	struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 8 June 2000

DESCRIPTION :
Updates the discretization text field. If there is a current element point,
writes the discretization in use, otherwise N/A.
==============================================================================*/
{
	char temp_string[60],*value_string;
	int dimension,is_editable,is_sensitive,*number_in_xi,return_code;
	struct FE_element *element;
 
	ENTER(Element_point_viewer_refresh_discretization_text);
	if (element_point_viewer)
	{
		return_code=1;
		if (value_string=
			XmTextFieldGetString(element_point_viewer->discretization_text))
		{
			if ((element=element_point_viewer->element_point_identifier.element)&&
				(dimension=get_FE_element_dimension(element))&&
				(number_in_xi=
					element_point_viewer->element_point_identifier.number_in_xi))
			{
				switch (dimension)
				{
					case 1:
					{
						sprintf(temp_string,"%d",number_in_xi[0]);
					} break;
					case 2:
					{
						sprintf(temp_string,"%d*%d",number_in_xi[0],number_in_xi[1]);
					} break;
					default:
					{
						sprintf(temp_string,"%d*%d*%d",number_in_xi[0],number_in_xi[1],
							number_in_xi[2]);
					} break;
				}
				is_sensitive=is_editable=
					(XI_DISCRETIZATION_EXACT_XI != element_point_viewer->
						element_point_identifier.xi_discretization_mode);
			}
			else
			{
				sprintf(temp_string,"N/A");
				is_editable=False;
				is_sensitive=False;
			}
			Element_point_viewer_set_textfield_edit_status(element_point_viewer,
				element_point_viewer->discretization_text,is_editable);
			/* only set string if different from that shown */
			if (strcmp(temp_string,value_string))
			{
				XmTextFieldSetString(element_point_viewer->discretization_text,
					temp_string);
			}
			XtSetSensitive(element_point_viewer->discretization_text,is_sensitive);
			XtFree(value_string);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_refresh_discretization_text.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_viewer_refresh_discretization_text */

static int Element_point_viewer_refresh_point_number_text(
	struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 8 June 2000

DESCRIPTION :
Updates the point_number text field. If there is a current element point,
writes its number, otherwise N/A.
==============================================================================*/
{
	char temp_string[20],*value_string;
	int return_code,is_editable,is_sensitive;
 
	ENTER(Element_point_viewer_refresh_point_number_text);
	if (element_point_viewer)
	{
		return_code=1;
		if (value_string=
			XmTextFieldGetString(element_point_viewer->point_number_text))
		{
			if (element_point_viewer->element_point_identifier.element)
			{
				sprintf(temp_string,"%d",element_point_viewer->element_point_number);
				is_sensitive=is_editable=
					(XI_DISCRETIZATION_EXACT_XI != element_point_viewer->
						element_point_identifier.xi_discretization_mode);
			}
			else
			{
				sprintf(temp_string,"N/A");
				is_editable=False;
				is_sensitive=False;
			}
			Element_point_viewer_set_textfield_edit_status(element_point_viewer,
				element_point_viewer->point_number_text,is_editable);
			/* only set string if different from that shown */
			if (strcmp(temp_string,value_string))
			{
				XmTextFieldSetString(element_point_viewer->point_number_text,
					temp_string);
			}
			XtSetSensitive(element_point_viewer->point_number_text,is_sensitive);
			XtFree(value_string);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_refresh_point_number_text.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_viewer_refresh_point_number_text */

static int Element_point_viewer_refresh_xi_text(
	struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 8 June 2000

DESCRIPTION :
Updates the xi text field. If there is a current element point, writes its xi
value otherwise N/A.
==============================================================================*/
{
	char temp_string[120],*value_string;
	FE_value *xi;
	int dimension,is_editable,is_sensitive,return_code;
	struct FE_element *element;
 
	ENTER(Element_point_viewer_refresh_xi_text);
	if (element_point_viewer)
	{
		return_code=1;
		if (value_string=XmTextFieldGetString(element_point_viewer->xi_text))
		{
			if ((element=element_point_viewer->element_point_identifier.element)&&
				(dimension=get_FE_element_dimension(element))&&
				(xi=element_point_viewer->xi))
			{
				switch (dimension)
				{
					case 1:
					{
						sprintf(temp_string,"%g",xi[0]);
					} break;
					case 2:
					{
						sprintf(temp_string,"%g, %g",xi[0],xi[1]);
					} break;
					default:
					{
						sprintf(temp_string,"%g, %g, %g",xi[0],xi[1],xi[2]);
					} break;
				}
				is_editable=(XI_DISCRETIZATION_EXACT_XI == element_point_viewer->
					element_point_identifier.xi_discretization_mode);
				is_sensitive=True;
			}
			else
			{
				sprintf(temp_string,"N/A");
				is_editable=False;
				is_sensitive=False;
			}
			Element_point_viewer_set_textfield_edit_status(element_point_viewer,
				element_point_viewer->xi_text,is_editable);
			/* only set string if different from that shown */
			if (strcmp(temp_string,value_string))
			{
				XmTextFieldSetString(element_point_viewer->xi_text,temp_string);
			}
			XtSetSensitive(element_point_viewer->xi_text,is_sensitive);
			XtFree(value_string);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_refresh_xi_text.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_viewer_refresh_xi_text */

static int Element_point_viewer_refresh_grid_value_text(
	struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 20 June 2000

DESCRIPTION :
Updates the grid_value text field. If there is a current element point, writes
the field value, otherwise N/A.
==============================================================================*/
{
	char *field_value_string,*value_string;
	int is_sensitive,return_code;
	struct Computed_field *grid_field;
	struct FE_element *element,*top_level_element;
 
	ENTER(Element_point_viewer_refresh_grid_value_text);
	if (element_point_viewer)
	{
		return_code=1;
		/* Get the text string */
		if (value_string=
			XmTextFieldGetString(element_point_viewer->grid_value_text))
		{
			if ((element=element_point_viewer->element_point_identifier.element)&&
				(top_level_element=
					element_point_viewer->element_point_identifier.top_level_element)&&
				(grid_field=CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
					element_point_viewer->grid_field_widget))&&
				Computed_field_is_defined_in_element(grid_field,element))
			{
				if (field_value_string=Computed_field_evaluate_as_string_in_element(
					grid_field,/*component_number*/-1,element,
					element_point_viewer->xi,top_level_element))
				{
					/* only set string from field if different from that shown */
					if (strcmp(field_value_string,value_string))
					{
						XmTextFieldSetString(element_point_viewer->grid_value_text,
							field_value_string);
					}
					DEALLOCATE(field_value_string);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Element_point_viewer_refresh_grid_value_text.  "
						"Could not evaluate field");
					XmTextFieldSetString(element_point_viewer->grid_value_text,"ERROR");
				}
				Computed_field_clear_cache(grid_field);
				is_sensitive=True;
			}
			else
			{
				/* only set string if different from that shown */
				if (strcmp(value_string,"N/A"))
				{
					XmTextFieldSetString(element_point_viewer->grid_value_text,"N/A");
				}
				is_sensitive=False;
			}
			XtSetSensitive(element_point_viewer->grid_value_text,is_sensitive);
			XtFree(value_string);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_refresh_grid_value_text.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_viewer_refresh_grid_value_text */

static int Element_point_viewer_refresh_chooser_widgets(
	struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 29 May 2000

DESCRIPTION :
Fills the widgets for choosing the element point with the current values.
==============================================================================*/
{
	int return_code;
 
	ENTER(Element_point_viewer_refresh_chooser_widgets);
	if (element_point_viewer)
	{
		return_code=1;
		Element_point_viewer_refresh_elements(element_point_viewer);
		Element_point_viewer_refresh_xi_discretization_mode(element_point_viewer);
		Element_point_viewer_refresh_discretization_text(element_point_viewer);
		Element_point_viewer_refresh_point_number_text(element_point_viewer);
		Element_point_viewer_refresh_xi_text(element_point_viewer);
		Element_point_viewer_refresh_grid_value_text(element_point_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_refresh_chooser_widgets.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_viewer_refresh_chooser_widgets */

static int Element_point_viewer_refresh_match_grid_field(
	struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 20 June 2000

DESCRIPTION :
Makes the button & chooser widgets reflect the current values of the
match_grid_field.
==============================================================================*/
{
	int field_set,return_code;
 
	ENTER(Element_point_viewer_refresh_match_grid_field);
	if (element_point_viewer)
	{
		return_code=1;
		/* make sure the current field is shown, if any */
		if (element_point_viewer->match_grid_field&&
			(element_point_viewer->match_grid_field !=
				CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
					element_point_viewer->match_grid_field_widget)))
		{
			if (!CHOOSE_OBJECT_SET_OBJECT(Computed_field)(
				element_point_viewer->match_grid_field_widget,
				element_point_viewer->match_grid_field))
			{
				element_point_viewer->match_grid_field=
					(struct Computed_field *)NULL;
			}
		}
		/* give the button and chooser the correct appearance */
		field_set=((struct Computed_field *)NULL !=
			element_point_viewer->match_grid_field);
		XmToggleButtonSetState(element_point_viewer->match_grid_button,
			/*state*/field_set,/*notify*/False);
		XtSetSensitive(element_point_viewer->match_grid_field_widget,
			field_set);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_refresh_match_grid_field.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_viewer_refresh_match_grid_field */

static void Element_point_viewer_element_point_ranges_selection_change(
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct Element_point_ranges_selection_changes *changes,
	void *element_point_viewer_void)
/*******************************************************************************
LAST MODIFIED : 25 May 2000

DESCRIPTION :
Callback for change in the global element_point selection.
==============================================================================*/
{
	int start,stop;
	struct Element_point_ranges *element_point_ranges;
	struct Element_point_viewer *element_point_viewer;
	struct Multi_range *ranges;

	ENTER(Element_point_viewer_element_point_ranges_selection_change);
	if (element_point_ranges_selection&&changes&&(element_point_viewer=
		(struct Element_point_viewer *)element_point_viewer_void))
	{
		/* get the last selected element_point and put it in the viewer */
		if ((element_point_ranges=
			FIRST_OBJECT_IN_LIST_THAT(Element_point_ranges)(
				(LIST_CONDITIONAL_FUNCTION(Element_point_ranges) *)NULL,(void *)NULL,
				changes->newly_selected_element_point_ranges_list))||
			(element_point_ranges=
				FIND_BY_IDENTIFIER_IN_LIST(Element_point_ranges,identifier)(
					&(element_point_viewer->element_point_identifier),
					Element_point_ranges_selection_get_element_point_ranges_list(
						element_point_ranges_selection)))||
			(element_point_ranges=
				FIRST_OBJECT_IN_LIST_THAT(Element_point_ranges)(
					(LIST_CONDITIONAL_FUNCTION(Element_point_ranges) *)NULL,(void *)NULL,
					Element_point_ranges_selection_get_element_point_ranges_list(
						element_point_ranges_selection))))
		{
			Element_point_ranges_get_identifier(element_point_ranges,
				&(element_point_viewer->element_point_identifier));
			if ((ranges=Element_point_ranges_get_ranges(element_point_ranges))&&
				Multi_range_get_range(ranges,0,&start,&stop))
			{
				element_point_viewer->element_point_number=start;
			}
			else
			{
				element_point_viewer->element_point_number=0;
			}
			Element_point_viewer_calculate_xi(element_point_viewer);
			Element_point_viewer_refresh_chooser_widgets(element_point_viewer);
			Element_point_viewer_set_viewer_element_point(element_point_viewer);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_element_point_ranges_selection_change.  "
			"Invalid argument(s)");
	}
	LEAVE;
} /* Element_point_viewer_element_point_ranges_selection_change */

static void Element_point_viewer_update_element(Widget widget,
	void *element_point_viewer_void,void *element_void)
/*******************************************************************************
LAST MODIFIED : 21 June 2000

DESCRIPTION :
Callback for change of element.
==============================================================================*/
{
	FE_value element_to_top_level[9];
	struct Element_point_viewer *element_point_viewer;
	struct FE_element *element;

	ENTER(Element_point_viewer_update_element);
	USE_PARAMETER(widget);
	if (element_point_viewer=
		(struct Element_point_viewer *)element_point_viewer_void)
	{
		/* don't select elements until there is a top_level_element */
		if (element_point_viewer->element_point_identifier.top_level_element&&
			(element=(struct FE_element *)element_void))
		{
			element_point_viewer->element_point_identifier.element=element;
			if (element)
			{
				/* get top_level_element, keeping existing one if possible */
				element_point_viewer->element_point_identifier.top_level_element=
					FE_element_get_top_level_element_conversion(
						element_point_viewer->element_point_identifier.element,
						element_point_viewer->element_point_identifier.top_level_element,
						(struct GROUP(FE_element) *)NULL,/*face_number*/-1,
						element_to_top_level);
				TEXT_CHOOSE_OBJECT_SET_OBJECT(FE_element)(
					element_point_viewer->top_level_element_widget,
					element_point_viewer->element_point_identifier.top_level_element);
				element_point_viewer->element_point_number=0;
				if (XI_DISCRETIZATION_CELL_CORNERS==
					element_point_viewer->element_point_identifier.xi_discretization_mode)
				{
					Element_point_viewer_get_grid(element_point_viewer);
				}
				Element_point_viewer_calculate_xi(element_point_viewer);
				Element_point_viewer_select_current_point(element_point_viewer);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_update_element.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_point_viewer_update_element */

static void Element_point_viewer_update_top_level_element(Widget widget,
	void *element_point_viewer_void,void *top_level_element_void)
/*******************************************************************************
LAST MODIFIED : 6 December 2000

DESCRIPTION :
Callback for change of top_level_element.
==============================================================================*/
{
	int i;
	struct Element_point_viewer *element_point_viewer;
	struct FE_element *top_level_element;

	ENTER(Element_point_viewer_update_top_level_element);
	USE_PARAMETER(widget);
	if (element_point_viewer=
		(struct Element_point_viewer *)element_point_viewer_void)
	{
		if (top_level_element=(struct FE_element *)top_level_element_void)
		{
			if (element_point_viewer->element_point_identifier.element)
			{
				if (top_level_element&&
					FE_element_is_top_level_parent_of_element(top_level_element,
						(void *)element_point_viewer->element_point_identifier.element))
				{
					element_point_viewer->element_point_identifier.top_level_element=
						top_level_element;
				}
				else
				{
					element_point_viewer->element_point_identifier.top_level_element=
						FIRST_OBJECT_IN_MANAGER_THAT(FE_element)(
							FE_element_is_top_level_parent_of_element,
							(void *)element_point_viewer->element_point_identifier.element,
							element_point_viewer->element_manager);
					TEXT_CHOOSE_OBJECT_SET_OBJECT(FE_element)(
						element_point_viewer->top_level_element_widget,
						element_point_viewer->element_point_identifier.top_level_element);
				}
			}
			else
			{
				/* use the top_level_element for the element too */
				element_point_viewer->element_point_identifier.element=
					top_level_element;
				TEXT_CHOOSE_OBJECT_SET_OBJECT(FE_element)(
					element_point_viewer->element_widget,
					element_point_viewer->element_point_identifier.element);
				element_point_viewer->element_point_identifier.top_level_element=
					top_level_element;
				/* get the element point at the centre of top_level_element */
				element_point_viewer->element_point_identifier.xi_discretization_mode=
					XI_DISCRETIZATION_EXACT_XI;
				for (i=0;i<MAXIMUM_ELEMENT_XI_DIMENSIONS;i++)
				{
					element_point_viewer->element_point_identifier.number_in_xi[i]=1;
					element_point_viewer->xi[i]=
						element_point_viewer->element_point_identifier.exact_xi[i]=0.5;
				}
				element_point_viewer->element_point_number=0;
				/* try to replace with a grid point, if possible */
				Element_point_viewer_get_grid(element_point_viewer);
			}
			Element_point_viewer_select_current_point(element_point_viewer);
		}
		else
		{
			/* must be no element if no top_level_element */
			element_point_viewer->element_point_identifier.element =
				(struct FE_element *)NULL;
			Element_point_viewer_refresh_xi_discretization_mode(element_point_viewer);
			Element_point_viewer_refresh_discretization_text(element_point_viewer);
			Element_point_viewer_refresh_point_number_text(element_point_viewer);
			Element_point_viewer_refresh_xi_text(element_point_viewer);
			Element_point_viewer_refresh_grid_value_text(element_point_viewer);
			Element_point_viewer_set_viewer_element_point(element_point_viewer);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_update_top_level_element.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_point_viewer_update_top_level_element */

static void Element_point_viewer_update_xi_discretization_mode(Widget widget,
	void *element_point_viewer_void,void *xi_discretization_mode_string_void)
/*******************************************************************************
LAST MODIFIED : 19 March 2001

DESCRIPTION :
Callback for change of xi_discretization_mode.
==============================================================================*/
{
	enum Xi_discretization_mode xi_discretization_mode;
	int i;
	struct Element_point_ranges_identifier temp_element_point_identifier;
	struct Element_point_viewer *element_point_viewer;

	ENTER(Element_point_viewer_update_xi_discretization_mode);
	USE_PARAMETER(widget);
	if (element_point_viewer=
		(struct Element_point_viewer *)element_point_viewer_void)
	{
		/* store old identifier unless in case new one is invalid */
		COPY(Element_point_ranges_identifier)(&temp_element_point_identifier,
			&(element_point_viewer->element_point_identifier));
		if (STRING_TO_ENUMERATOR(Xi_discretization_mode)(
			(char *)xi_discretization_mode_string_void, &xi_discretization_mode))
		{
			element_point_viewer->element_point_identifier.xi_discretization_mode=
				xi_discretization_mode;
			if (XI_DISCRETIZATION_CELL_CORNERS==xi_discretization_mode)
			{
				Element_point_viewer_get_grid(element_point_viewer);
			}
			else if (XI_DISCRETIZATION_EXACT_XI==xi_discretization_mode)
			{
				for (i=0;i<MAXIMUM_ELEMENT_XI_DIMENSIONS;i++)
				{
					element_point_viewer->element_point_identifier.number_in_xi[i]=1;
				}
			}
			element_point_viewer->element_point_number=0;
			if (Element_point_ranges_identifier_is_valid(
				&(element_point_viewer->element_point_identifier)))
			{
				Element_point_viewer_calculate_xi(element_point_viewer);
				Element_point_viewer_select_current_point(element_point_viewer);
			}
			else
			{
				COPY(Element_point_ranges_identifier)(
					&(element_point_viewer->element_point_identifier),
					&temp_element_point_identifier);
				/* always restore mode to actual value in use */
				Element_point_viewer_refresh_xi_discretization_mode(
					element_point_viewer);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_update_xi_discretization_mode.  "
			"Invalid argument(s)");
	}
	LEAVE;
} /* Element_point_viewer_update_xi_discretization_mode */

static void Element_point_viewer_discretization_text_CB(Widget widget,
	void *element_point_viewer_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 30 May 2000

DESCRIPTION :
Called when entry is made into the discretization_text field.
==============================================================================*/
{
	char *value_string;
	struct Element_discretization discretization,temp_discretization;
	struct Element_point_viewer *element_point_viewer;
	struct Parse_state *temp_state;
	XmAnyCallbackStruct *any_callback;

	ENTER(Element_point_viewer_discretization_text_CB);
	USE_PARAMETER(widget);
	if ((element_point_viewer=
		(struct Element_point_viewer *)element_point_viewer_void)&&
		(any_callback=(XmAnyCallbackStruct *)call_data))
	{
		if (XmCR_ACTIVATE == any_callback->reason)
		{
			/* Get the text string */
			if (value_string=
				XmTextFieldGetString(element_point_viewer->discretization_text))
			{
				if (temp_state=create_Parse_state(value_string))
				{
					temp_discretization.number_in_xi1=
						element_point_viewer->element_point_identifier.number_in_xi[0];
					temp_discretization.number_in_xi2=
						element_point_viewer->element_point_identifier.number_in_xi[1];
					temp_discretization.number_in_xi3=
						element_point_viewer->element_point_identifier.number_in_xi[2];
					if (set_Element_discretization(temp_state,(void *)&discretization,
						(void *)element_point_viewer->user_interface))
					{
						element_point_viewer->element_point_identifier.number_in_xi[0]=
							discretization.number_in_xi1;
						element_point_viewer->element_point_identifier.number_in_xi[1]=
							discretization.number_in_xi2;
						element_point_viewer->element_point_identifier.number_in_xi[2]=
							discretization.number_in_xi3;
						if (Element_point_ranges_identifier_is_valid(
							&(element_point_viewer->element_point_identifier)))
						{
							element_point_viewer->element_point_number=0;
							Element_point_viewer_select_current_point(element_point_viewer);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Element_point_viewer_discretization_text_CB.  "
								"Invalid element point");
							element_point_viewer->element_point_identifier.number_in_xi[0]=
								temp_discretization.number_in_xi1;
							element_point_viewer->element_point_identifier.number_in_xi[1]=
								temp_discretization.number_in_xi2;
							element_point_viewer->element_point_identifier.number_in_xi[2]=
								temp_discretization.number_in_xi3;
						}
					}
					destroy_Parse_state(&temp_state);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Element_point_viewer_discretization_text_CB.  "
						"Could not create parse state");
				}
				XtFree(value_string);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Element_point_viewer_discretization_text_CB.  Missing text");
			}
		}
		/* always restore discretization_text to actual value stored */
		Element_point_viewer_refresh_discretization_text(element_point_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_discretization_text_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_point_viewer_discretization_text_CB */

static void Element_point_viewer_point_number_text_CB(Widget widget,
	void *element_point_viewer_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 30 May 2000

DESCRIPTION :
Called when entry is made into the point_number_text field.
==============================================================================*/
{
	char *value_string;
	int element_point_number;
	struct Element_point_viewer *element_point_viewer;
	XmAnyCallbackStruct *any_callback;

	ENTER(Element_point_viewer_point_number_text_CB);
	USE_PARAMETER(widget);
	if ((element_point_viewer=
		(struct Element_point_viewer *)element_point_viewer_void)&&
		(any_callback=(XmAnyCallbackStruct *)call_data))
	{
		if (XmCR_ACTIVATE == any_callback->reason)
		{
			/* Get the text string */
			if (value_string=
				XmTextFieldGetString(element_point_viewer->point_number_text))
			{
				if ((1==sscanf(value_string,"%d",&element_point_number))&&
					Element_point_ranges_identifier_element_point_number_is_valid(
						&(element_point_viewer->element_point_identifier),
						element_point_number))
				{
					element_point_viewer->element_point_number=element_point_number;
					Element_point_viewer_select_current_point(element_point_viewer);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Element_point_viewer_point_number_text_CB.  Invalid point number");
				}
				XtFree(value_string);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Element_point_viewer_point_number_text_CB.  Missing text");
			}
		}
		/* always restore point_number_text to actual value stored */
		Element_point_viewer_refresh_point_number_text(element_point_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_point_number_text_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_point_viewer_point_number_text_CB */

static void Element_point_viewer_xi_text_CB(Widget widget,
	void *element_point_viewer_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 6 December 2000

DESCRIPTION :
Called when entry is made into the xi_text field.
==============================================================================*/
{
	char *value_string;
	float xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int dimension,i;
	struct Element_point_viewer *element_point_viewer;
	struct FE_element *element;
	struct Parse_state *temp_state;
	XmAnyCallbackStruct *any_callback;

	ENTER(Element_point_viewer_xi_text_CB);
	USE_PARAMETER(widget);
	if ((element_point_viewer=
		(struct Element_point_viewer *)element_point_viewer_void) &&
		(any_callback = (XmAnyCallbackStruct *)call_data))
	{
		if ((element = element_point_viewer->element_point_identifier.element) &&
			(dimension = get_FE_element_dimension(element)))
		{
			if (XmCR_ACTIVATE == any_callback->reason)
			{
				/* Get the text string */
				if (value_string=
					XmTextFieldGetString(element_point_viewer->xi_text))
				{
					/* clean up spaces? */
					if (temp_state=create_Parse_state(value_string))
					{
						if (set_float_vector(temp_state,xi,(void *)&dimension))
						{
							for (i=0;i<dimension;i++)
							{
								element_point_viewer->element_point_identifier.exact_xi[i] =
									xi[i];
							}
							Element_point_viewer_select_current_point(element_point_viewer);
						}
						destroy_Parse_state(&temp_state);
					}
					XtFree(value_string);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Element_point_viewer_xi_text_CB.  Missing text");
				}
			}
		}
		/* always restore xi_text to actual value stored */
		Element_point_viewer_refresh_xi_text(element_point_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_xi_text_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_point_viewer_xi_text_CB */

static void Element_point_viewer_update_grid_field(Widget widget,
	void *element_point_viewer_void,void *grid_field_void)
/*******************************************************************************
LAST MODIFIED : 26 May 2000

DESCRIPTION :
Callback for change of grid field.
==============================================================================*/
{
	struct Element_point_viewer *element_point_viewer;

	ENTER(Element_point_viewer_update_grid_field);
	USE_PARAMETER(widget);
	USE_PARAMETER(grid_field_void);
	if (element_point_viewer=
		(struct Element_point_viewer *)element_point_viewer_void)
	{
		Element_point_viewer_refresh_grid_value_text(element_point_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_update_grid_field.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_point_viewer_update_grid_field */

static void Element_point_viewer_grid_value_text_CB(Widget widget,
	void *element_point_viewer_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 30 May 2000

DESCRIPTION :
Called when entry is made into the grid_value_text field.
==============================================================================*/
{
	char *value_string;
	int grid_value;
	struct Computed_field *grid_field;
	struct Element_point_viewer *element_point_viewer;
	struct FE_element_grid_to_Element_point_ranges_list_data grid_to_list_data;
	struct FE_field *grid_fe_field;
	XmAnyCallbackStruct *any_callback;

	ENTER(Element_point_viewer_grid_value_text_CB);
	USE_PARAMETER(widget);
	if ((element_point_viewer=
		(struct Element_point_viewer *)element_point_viewer_void)&&
		(any_callback=(XmAnyCallbackStruct *)call_data))
	{
		if (XmCR_ACTIVATE == any_callback->reason)
		{
			/* Get the text string */
			if (value_string=
				XmTextFieldGetString(element_point_viewer->grid_value_text))
			{
				if ((grid_field=CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
					element_point_viewer->grid_field_widget))&&
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
									element_point_viewer->element_manager);
								if (0<NUMBER_IN_LIST(Element_point_ranges)(
									grid_to_list_data.element_point_ranges_list))
								{
									Element_point_ranges_selection_begin_cache(
										element_point_viewer->element_point_ranges_selection);
									Element_point_ranges_selection_clear(
										element_point_viewer->element_point_ranges_selection);
									FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
										Element_point_ranges_select,
										(void *)element_point_viewer->element_point_ranges_selection,
										grid_to_list_data.element_point_ranges_list);
									Element_point_ranges_selection_end_cache(
										element_point_viewer->element_point_ranges_selection);
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
					"Element_point_viewer_grid_value_text_CB.  Missing text");
			}
		}
		/* always restore grid_value_text to actual value stored */
		Element_point_viewer_refresh_grid_value_text(element_point_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_grid_value_text_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_point_viewer_grid_value_text_CB */

static void Element_point_viewer_apply_options_button_CB(Widget widget,
	void *element_point_viewer_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 20 June 2000

DESCRIPTION :
Toggles visibility of apply options panel.
==============================================================================*/
{
	struct Element_point_viewer *element_point_viewer;

	ENTER(Element_point_viewer_apply_options_button_CB);
	USE_PARAMETER(call_data);
	if (widget&&(element_point_viewer=
		(struct Element_point_viewer *)element_point_viewer_void))
	{
		if (XmToggleButtonGetState(widget))
		{
			XtManageChild(element_point_viewer->apply_options);
		}
		else
		{
			XtUnmanageChild(element_point_viewer->apply_options);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_apply_options_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_point_viewer_apply_options_button_CB */

static void Element_point_viewer_match_grid_button_CB(Widget widget,
	void *element_point_viewer_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 20 June 2000

DESCRIPTION :
Toggles the use of a match grid field when applying the changes made in the
viewer. When set, this field applies the same modified values to all points
with the same value of this field. Used chiefly to ensure that values set at
grid points on common element boundaries are the same for the elements on either
side.
==============================================================================*/
{
	struct Element_point_viewer *element_point_viewer;

	ENTER(Element_point_viewer_match_grid_button_CB);
	USE_PARAMETER(call_data);
	if (widget&&(element_point_viewer=
		(struct Element_point_viewer *)element_point_viewer_void))
	{
		if (element_point_viewer->match_grid_field)
		{
			element_point_viewer->match_grid_field=(struct Computed_field *)NULL;
		}
		else
		{
			/* get grid field from widget */
			element_point_viewer->match_grid_field=
				CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
					element_point_viewer->match_grid_field_widget);
		}
		Element_point_viewer_refresh_match_grid_field(element_point_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_match_grid_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_point_viewer_match_grid_button_CB */

static void Element_point_viewer_update_match_grid_field(Widget widget,
	void *element_point_viewer_void,void *match_grid_field_void)
/*******************************************************************************
LAST MODIFIED : 20 June 2000

DESCRIPTION :
Callback for change of match grid field.
==============================================================================*/
{
	struct Element_point_viewer *element_point_viewer;

	ENTER(Element_point_viewer_update_match_grid_field);
	USE_PARAMETER(widget);
	if (element_point_viewer=
		(struct Element_point_viewer *)element_point_viewer_void)
	{
		/* skip messages from chooser if grayed out */
		if (element_point_viewer->match_grid_field)
		{
			element_point_viewer->match_grid_field=
				(struct Computed_field *)match_grid_field_void;
			Element_point_viewer_refresh_match_grid_field(element_point_viewer);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_update_match_grid_field.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_point_viewer_update_match_grid_field */

static int Element_point_viewer_apply_changes(
	struct Element_point_viewer *element_point_viewer,int apply_all)
/*******************************************************************************
LAST MODIFIED : 22 June 2000

DESCRIPTION :
Makes the element point change global. If <apply_all> then apply the changes to
all element points in the global selection. Note that values can only be
applied to element points that are grid_points - warnings will be given if
attempts are made to apply changes at element points in other locations.
Furthermore, if the <element_point_viewer>
==============================================================================*/
{
	char *field_name;
	int return_code;
	struct Element_point_ranges_grid_to_multi_range_data grid_to_multi_range_data;
	struct Element_point_ranges_identifier source_identifier;
	struct Element_point_ranges_set_grid_values_data set_grid_values_data;
	struct FE_element_grid_to_Element_point_ranges_list_data grid_to_list_data;
	struct LIST(Element_point_ranges) *element_point_ranges_list;

	ENTER(Element_point_viewer_apply_changes);
	if (element_point_viewer&&
		element_point_viewer->element_point_identifier.element&&
		element_point_viewer->element_copy)
	{
		return_code=1;
		/* only apply if new values have been entered for field components */
		if (0<NUMBER_IN_LIST(Field_value_index_ranges)(
			element_point_viewer->modified_field_components))
		{
			if (element_point_ranges_list=CREATE(LIST(Element_point_ranges))())
			{
				/* put current element point in list */
				return_code=
					Element_point_ranges_list_add_element_point(element_point_ranges_list,
						&(element_point_viewer->element_point_identifier),
						element_point_viewer->element_point_number);
				/* if apply_all, add all other selected element points */
				if (return_code&&apply_all)
				{
					FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
						Element_point_ranges_add_to_list,(void *)element_point_ranges_list,
						Element_point_ranges_selection_get_element_point_ranges_list(
							element_point_viewer->element_point_ranges_selection));
				}
				/* if match_grid_field, get range of its values for all points in
					 element_point_ranges_list, then get list of all grid-points with
					 those values. Usually this is the grid_point_number field, and this
					 feature is used to overcome the fact that we store values for same-
					 numbered grid-point more than once on common element boundaries */
				if (return_code&&element_point_viewer->match_grid_field)
				{
					/* check field is wrapper for single component integer FE_field */
					if (Computed_field_get_type_finite_element(
						element_point_viewer->match_grid_field,
						&(grid_to_multi_range_data.grid_fe_field))&&
						(1==get_FE_field_number_of_components(
							grid_to_multi_range_data.grid_fe_field))&&
						(INT_VALUE==get_FE_field_value_type(
							grid_to_multi_range_data.grid_fe_field)))
					{
						/* get multi-range of values of match_grid_field for points in
							 element_point_ranges_list */
						if (grid_to_multi_range_data.multi_range=CREATE(Multi_range)())
						{
							/* if following flag is cleared it means that some of the selected
								 element points are not grid points */
							grid_to_multi_range_data.all_points_native=1;
							return_code=FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
								Element_point_ranges_grid_to_multi_range,
								(void *)&grid_to_multi_range_data,element_point_ranges_list);
#if defined (OLD_CODE)
							/* warning will be given later anyway */
							if (!grid_to_multi_range_data.all_points_native)
							{
								display_message(WARNING_MESSAGE,
									"Values can not be set at element points not on grid");
							}
#endif /* defined (OLD_CODE) */
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
								return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
									FE_element_grid_to_Element_point_ranges_list,
									(void *)&grid_to_list_data,
									element_point_viewer->element_manager);
							}
							DESTROY(Multi_range)(&(grid_to_multi_range_data.multi_range));
						}
						else
						{
							return_code=0;
						}
						if (!return_code)
						{
							GET_NAME(Computed_field)(
								element_point_viewer->match_grid_field,&field_name);
							display_message(WARNING_MESSAGE,
								"Element_point_viewer_apply_changes.  Could not set same values"
								"for points with same value of field %s",field_name);
							DEALLOCATE(field_name);
						}
					}
					else
					{
						display_message(WARNING_MESSAGE,
							"Element_point_viewer_apply_changes.  "
							"Invalid match_grid_field");
						return_code=0;
					}
				}
				if (return_code)
				{
					if (0<NUMBER_IN_LIST(Element_point_ranges)(element_point_ranges_list))
					{
						/* to modify, need the element point to take values from */
						if (return_code=
							COPY(Element_point_ranges_identifier)(&source_identifier,
								&(element_point_viewer->element_point_identifier)))
						{
							source_identifier.element=element_point_viewer->element_copy;
							/* note values taken from the local element_copy... */
							set_grid_values_data.source_identifier=&source_identifier;
							set_grid_values_data.source_element_point_number=
								element_point_viewer->element_point_number;
							/* need the components that have been modified ... */
							set_grid_values_data.field_component_ranges_list=
								element_point_viewer->modified_field_components;
							/* ... and the manager to modify them in */
							set_grid_values_data.element_manager=
								element_point_viewer->element_manager;
							/* if following flag is cleared it means that some of the selected
								 element points are not grid points */
							set_grid_values_data.all_points_native=1;
							/* cache manager modifies when more than one being changed */
							MANAGER_BEGIN_CACHE(FE_element)(
								element_point_viewer->element_manager);
							return_code=FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
								Element_point_ranges_set_grid_values,
								(void *)&set_grid_values_data,element_point_ranges_list);
							if (!set_grid_values_data.all_points_native)
							{
								display_message(WARNING_MESSAGE,
									"Values only set at element points on grid");
							}
							MANAGER_END_CACHE(FE_element)(
								element_point_viewer->element_manager);
						}
						if (!return_code)
						{
							display_message(ERROR_MESSAGE,
								"Element_point_viewer_apply_changes.  Could not set values");
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
				display_message(WARNING_MESSAGE,
					"Element_point_viewer_apply_changes.  Could not make list");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"Element_point_viewer_apply_changes.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_viewer_apply_changes */

static void Element_point_viewer_apply_CB(Widget widget,
	void *element_point_viewer_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 16 June 2000

DESCRIPTION :
Callback for Apply button. Applies changes to the element point in the editor.
==============================================================================*/
{
	struct Element_point_viewer *element_point_viewer;

	ENTER(Element_point_viewer_apply_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (element_point_viewer=
		(struct Element_point_viewer *)element_point_viewer_void)
	{
		Element_point_viewer_apply_changes(element_point_viewer,/*apply_all*/0);
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"Element_point_viewer_apply_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_point_viewer_apply_CB */

static void Element_point_viewer_apply_all_CB(Widget widget,
	void *element_point_viewer_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 16 June 2000

DESCRIPTION :
Callback for Apply all button. Applies changes to the element point in the
editor and all other selected element points.
==============================================================================*/
{
	struct Element_point_viewer *element_point_viewer;

	ENTER(Element_point_viewer_apply_all_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (element_point_viewer=
		(struct Element_point_viewer *)element_point_viewer_void)
	{
		Element_point_viewer_apply_changes(element_point_viewer,/*apply_all*/1);
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"Element_point_viewer_apply_all_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_point_viewer_apply_all_CB */

static void Element_point_viewer_revert_CB(Widget widget,
	void *element_point_viewer_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 16 June 2000

DESCRIPTION :
Callback for Revert button. Sends global element point values back to the
editor widget, undoing any modifications.
==============================================================================*/
{
	struct Element_point_viewer *element_point_viewer;

	ENTER(Element_point_viewer_revert_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (element_point_viewer=
		(struct Element_point_viewer *)element_point_viewer_void)
	{
		Element_point_viewer_set_viewer_element_point(element_point_viewer);
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"Element_point_viewer_revert_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_point_viewer_revert_CB */

static void Element_point_viewer_close_CB(Widget widget,
	void *element_point_viewer_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 16 June 2000

DESCRIPTION :
Callback from the Close button.
Also called when "close" is selected from the window menu, or it is double
clicked. How this is made to occur is as follows. The dialog has its
XmNdeleteResponse == XmDO_NOTHING, and a window manager protocol callback for
WM_DELETE_WINDOW has been set up with XmAddWMProtocolCallback to call this
function in response to the close command. See CREATE for more details.
==============================================================================*/
{
	struct Element_point_viewer *element_point_viewer;

	ENTER(Element_point_viewer_close_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (element_point_viewer=
		(struct Element_point_viewer *)element_point_viewer_void)
	{
		DESTROY(Element_point_viewer)(
			element_point_viewer->element_point_viewer_address);
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"Element_point_viewer_close_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_point_viewer_close_CB */

static void Element_point_viewer_element_change(
	struct MANAGER_MESSAGE(FE_element) *message,
	void *element_point_viewer_void)
/*******************************************************************************
LAST MODIFIED : 20 June 2000

DESCRIPTION :
Callback from the element manager for changes to elements. If the element
currently being viewed has changed, re-send to viewer.
Note that we do not have to handle add, delete and identifier change messages
here as the select widget does this for us. Only changes to the content of the
object cause updates.
==============================================================================*/
{
	struct Element_point_viewer *element_point_viewer;

	ENTER(Element_point_viewer_element_change);
	if (message&&(element_point_viewer=
		(struct Element_point_viewer *)element_point_viewer_void))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_ALL(FE_element):
			case MANAGER_CHANGE_OBJECT(FE_element):
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(FE_element):
			{
				if (!(message->object_changed) || (message->object_changed ==
					element_point_viewer->element_point_identifier.element))
				{
					/* update grid_text in case number changed */
					Element_point_viewer_refresh_grid_value_text(element_point_viewer);
					Element_point_viewer_set_viewer_element_point(element_point_viewer);
				}
			} break;
			case MANAGER_CHANGE_ADD(FE_element):
			case MANAGER_CHANGE_DELETE(FE_element):
			case MANAGER_CHANGE_IDENTIFIER(FE_element):
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_element_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_point_viewer_element_change */

static void Element_point_viewer_node_change(
	struct MANAGER_MESSAGE(FE_node) *message,
	void *element_point_viewer_void)
/*******************************************************************************
LAST MODIFIED : 31 May 2000

DESCRIPTION :
Callback from the node manager for changes to nodes. If the element
currently being viewed is affected by the change, re-send to viewer.
==============================================================================*/
{
	struct Element_point_viewer *element_point_viewer;

	ENTER(Element_point_viewer_node_change);
	if (message&&(element_point_viewer=
		(struct Element_point_viewer *)element_point_viewer_void))
	{
		if (element_point_viewer->element_point_identifier.element)
		{
			switch (message->change)
			{
				case MANAGER_CHANGE_ALL(FE_node):
				case MANAGER_CHANGE_OBJECT(FE_node):
				case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(FE_node):
				{
					if (!(message->object_changed)||FE_element_or_parent_contains_node(
						element_point_viewer->element_point_identifier.element,
						message->object_changed))
					{
						Element_point_viewer_set_viewer_element_point(element_point_viewer);
					}
				} break;
				case MANAGER_CHANGE_ADD(FE_node):
				case MANAGER_CHANGE_DELETE(FE_node):
				case MANAGER_CHANGE_IDENTIFIER(FE_node):
				{
					/* do nothing */
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_node_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_point_viewer_node_change */

/*
Global functions
----------------
*/

struct Element_point_viewer *CREATE(Element_point_viewer)(
	struct Element_point_viewer **element_point_viewer_address,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(FE_node) *node_manager,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct Computed_field_package *computed_field_package,
	struct MANAGER(FE_field) *fe_field_manager,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 30 June 2000

DESCRIPTION :
Creates a dialog for choosing element points and displaying and editing their
fields.
==============================================================================*/
{
	Atom WM_DELETE_WINDOW;
	char **valid_strings;
	int i,init_widgets,number_of_faces,number_of_valid_strings,start,stop,
		temp_element_point_number;
	MrmType element_point_viewer_dialog_class;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Callback_data callback;
	struct Element_point_ranges_identifier temp_element_point_identifier;
	struct Element_point_ranges *element_point_ranges;
	struct Element_point_viewer *element_point_viewer;
	struct Computed_field *grid_field;
	struct Multi_range *ranges;
	static MrmRegisterArg callback_list[]=
	{
		{"elem_pt_v_id_element_form",(XtPointer)
			DIALOG_IDENTIFY(element_point_viewer,element_form)},
		{"elem_pt_v_id_top_element_form",(XtPointer)
			DIALOG_IDENTIFY(element_point_viewer,top_level_element_form)},
		{"elem_pt_v_id_xi_disc_mode_form",(XtPointer)
			DIALOG_IDENTIFY(element_point_viewer,xi_discretization_mode_form)},
		{"elem_pt_v_id_disc_number_entry",(XtPointer)
			DIALOG_IDENTIFY(element_point_viewer,discretization_number_entry)},
		{"elem_pt_v_id_disc_text",(XtPointer)
			DIALOG_IDENTIFY(element_point_viewer,discretization_text)},
		{"elem_pt_v_id_point_number_text",(XtPointer)
			DIALOG_IDENTIFY(element_point_viewer,point_number_text)},
		{"elem_pt_v_id_xi_text",(XtPointer)
			DIALOG_IDENTIFY(element_point_viewer,xi_text)},
		{"elem_pt_v_id_grid_number_entry",(XtPointer)
			DIALOG_IDENTIFY(element_point_viewer,grid_number_entry)},
		{"elem_pt_v_id_grid_field_form",(XtPointer)
			DIALOG_IDENTIFY(element_point_viewer,grid_field_form)},
		{"elem_pt_v_id_grid_value_text",(XtPointer)
			DIALOG_IDENTIFY(element_point_viewer,grid_value_text)},
		{"elem_pt_v_id_viewer_form",(XtPointer)
			DIALOG_IDENTIFY(element_point_viewer,viewer_form)},
		{"elem_pt_v_id_apply_options_btn",(XtPointer)
			DIALOG_IDENTIFY(element_point_viewer,apply_options_button)},
		{"elem_pt_v_id_apply_options",(XtPointer)
			DIALOG_IDENTIFY(element_point_viewer,apply_options)},
		{"elem_pt_v_id_match_grid_btn",(XtPointer)
			DIALOG_IDENTIFY(element_point_viewer,match_grid_button)},
		{"elem_pt_v_id_match_field_form",(XtPointer)
			DIALOG_IDENTIFY(element_point_viewer,match_grid_field_form)},
		{"elem_pt_v_disc_text_CB",
		 (XtPointer)Element_point_viewer_discretization_text_CB},
		{"elem_pt_v_point_number_text_CB",
		 (XtPointer)Element_point_viewer_point_number_text_CB},
		{"elem_pt_v_xi_text_CB",
		 (XtPointer)Element_point_viewer_xi_text_CB},
		{"elem_pt_v_grid_value_text_CB",
		 (XtPointer)Element_point_viewer_grid_value_text_CB},
		{"elem_pt_v_apply_options_btn_CB",(XtPointer)
			Element_point_viewer_apply_options_button_CB},
		{"elem_pt_v_match_grid_btn_CB",(XtPointer)
			Element_point_viewer_match_grid_button_CB},
		{"elem_pt_v_apply_CB",(XtPointer)Element_point_viewer_apply_CB},
		{"elem_pt_v_apply_all_CB",(XtPointer)Element_point_viewer_apply_all_CB},
		{"elem_pt_v_revert_CB",(XtPointer)Element_point_viewer_revert_CB},
		{"elem_pt_v_close_CB",(XtPointer)Element_point_viewer_close_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"elem_pt_v_structure",(XtPointer)NULL}
	};

	ENTER(CREATE(Element_point_viewer));
	element_point_viewer=(struct Element_point_viewer *)NULL;
	if (element_point_viewer_address&&element_manager&&node_manager&&
		element_point_ranges_selection&&computed_field_package&&
		(computed_field_manager=Computed_field_package_get_computed_field_manager(
			computed_field_package))&&fe_field_manager&&user_interface)
	{
		if (MrmOpenHierarchy_base64_string(element_point_viewer_uidh,
			&element_point_viewer_hierarchy,&element_point_viewer_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(element_point_viewer,struct Element_point_viewer,1)&&
				(element_point_viewer->modified_field_components=
					CREATE(LIST(Field_value_index_ranges))()))
			{
				element_point_viewer->computed_field_package=computed_field_package;
				element_point_viewer->element_point_viewer_address=
					element_point_viewer_address;
				element_point_viewer->element_manager=element_manager;
				element_point_viewer->element_manager_callback_id=(void *)NULL;
				element_point_viewer->node_manager=node_manager;
				element_point_viewer->node_manager_callback_id=(void *)NULL;
				element_point_viewer->element_point_ranges_selection=
					element_point_ranges_selection;
				element_point_viewer->user_interface=user_interface;
				element_point_viewer->element_point_identifier.element=
					(struct FE_element *)NULL;
				element_point_viewer->element_point_identifier.top_level_element=
					(struct FE_element *)NULL;
				element_point_viewer->element_copy=(struct FE_element *)NULL;
				element_point_viewer->element_point_identifier.xi_discretization_mode=
					XI_DISCRETIZATION_EXACT_XI;
				for (i=0;i<MAXIMUM_ELEMENT_XI_DIMENSIONS;i++)
				{
					element_point_viewer->element_point_identifier.number_in_xi[i]=1;
					element_point_viewer->xi[i]=
						element_point_viewer->element_point_identifier.exact_xi[i]=0.5;
				}
				element_point_viewer->element_point_number=0;
				element_point_viewer->match_grid_field=
					FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
						"grid_point_number",computed_field_manager);
				/* initialise widgets */
				element_point_viewer->element_form=(Widget)NULL;
				element_point_viewer->element_widget=(Widget)NULL;
				element_point_viewer->top_level_element_form=(Widget)NULL;
				element_point_viewer->top_level_element_widget=(Widget)NULL;
				element_point_viewer->xi_discretization_mode_form=(Widget)NULL;
				element_point_viewer->xi_discretization_mode_widget=(Widget)NULL;
				element_point_viewer->discretization_number_entry=(Widget)NULL;
				element_point_viewer->discretization_text=(Widget)NULL;
				element_point_viewer->point_number_text=(Widget)NULL;
				element_point_viewer->xi_text=(Widget)NULL;
				element_point_viewer->grid_number_entry=(Widget)NULL;
				element_point_viewer->grid_field_form=(Widget)NULL;
				element_point_viewer->grid_field_widget=(Widget)NULL;
				element_point_viewer->grid_value_text=(Widget)NULL;
				element_point_viewer->viewer_form=(Widget)NULL;
				element_point_viewer->viewer_widget=(Widget)NULL;
				element_point_viewer->apply_options_button=(Widget)NULL;
				element_point_viewer->apply_options=(Widget)NULL;
				element_point_viewer->match_grid_button=(Widget)NULL;
				element_point_viewer->match_grid_field_form=(Widget)NULL;
				element_point_viewer->match_grid_field_widget=(Widget)NULL;
				element_point_viewer->widget=(Widget)NULL;
				element_point_viewer->window_shell=(Widget)NULL;
				/* initialise the structure */
				if (element_point_ranges=FIRST_OBJECT_IN_LIST_THAT(Element_point_ranges)
					((LIST_CONDITIONAL_FUNCTION(Element_point_ranges) *)NULL,(void *)NULL,
						Element_point_ranges_selection_get_element_point_ranges_list(
							element_point_ranges_selection)))
				{
					Element_point_ranges_get_identifier(element_point_ranges,
						&(element_point_viewer->element_point_identifier));
					if ((ranges=Element_point_ranges_get_ranges(element_point_ranges))&&
						Multi_range_get_range(ranges,0,&start,&stop))
					{
						element_point_viewer->element_point_number=start;
					}
				}
				else
				{
					if (element_point_viewer->element_point_identifier.element=
						FIRST_OBJECT_IN_MANAGER_THAT(FE_element)(
							FE_element_is_top_level,(void *)NULL,element_manager))
					{
						element_point_viewer->element_point_identifier.top_level_element=
							element_point_viewer->element_point_identifier.element;
					}
					else
					{
						element_point_viewer->element_point_identifier.top_level_element=
							(struct FE_element *)NULL;
					}
					Element_point_viewer_get_grid(element_point_viewer);
					Element_point_viewer_select_current_point(element_point_viewer);
				}
				Element_point_viewer_calculate_xi(element_point_viewer);
				if (element_point_viewer->element_point_identifier.top_level_element)
				{
					if (element_point_viewer->element_copy=ACCESS(FE_element)(
						CREATE(FE_element)(element_point_viewer->
							element_point_identifier.top_level_element->identifier,
							element_point_viewer->
							element_point_identifier.top_level_element)))
					{
						/* clear the faces of element_copy as messes up exterior
							 calculations for graphics created from them */
						number_of_faces=
							element_point_viewer->element_copy->shape->number_of_faces;
						for (i=0;i<number_of_faces;i++)
						{
							set_FE_element_face(element_point_viewer->element_copy,i,
								(struct FE_element *)NULL);
						}
					}
				}
				else
				{
					element_point_viewer->element_copy=(struct FE_element *)NULL;
				}
				/* get callbacks from global element_point selection */
				Element_point_ranges_selection_add_callback(
					element_point_ranges_selection,
					Element_point_viewer_element_point_ranges_selection_change,
					(void *)element_point_viewer);
				/* make the dialog shell */
				if (element_point_viewer->window_shell=
					XtVaCreatePopupShell("element_point_viewer",
						topLevelShellWidgetClass,
						user_interface->application_shell,
						XmNdeleteResponse,XmDO_NOTHING,
						XmNmwmDecorations,MWM_DECOR_ALL,
						XmNmwmFunctions,MWM_FUNC_ALL,
						/*XmNtransient,FALSE,*/
						XmNallowShellResize,False,
						XmNtitle,"Element Point Viewer",
						NULL))
				{
					/* Set up window manager callback for close window message */
					WM_DELETE_WINDOW=
						XmInternAtom(XtDisplay(element_point_viewer->window_shell),
							"WM_DELETE_WINDOW",False);
					XmAddWMProtocolCallback(element_point_viewer->window_shell,
						WM_DELETE_WINDOW,Element_point_viewer_close_CB,
						element_point_viewer);
					/* Register the shell with the busy signal list */
					create_Shell_list_item(&(element_point_viewer->window_shell),
						user_interface);
					/* register the callbacks */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						element_point_viewer_hierarchy,callback_list,
						XtNumber(callback_list)))
					{
						/* assign and register the identifiers */
						identifier_list[0].value=(XtPointer)element_point_viewer;
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
							element_point_viewer_hierarchy,identifier_list,
							XtNumber(identifier_list)))
						{
							/* fetch position window widget */
							if (MrmSUCCESS==MrmFetchWidget(element_point_viewer_hierarchy,
								"element_point_viewer",element_point_viewer->window_shell,
								&(element_point_viewer->widget),
								&element_point_viewer_dialog_class))
							{
								/* get colours for editable and non-editable text fields -
									 make non_editable look like labels */
								XtVaGetValues(element_point_viewer->xi_text,XmNforeground,
									&(element_point_viewer->textfield_foreground_color),NULL);
								XtVaGetValues(element_point_viewer->xi_text,XmNbackground,
									&(element_point_viewer->textfield_background_color),NULL);
								XtVaGetValues(element_point_viewer->xi_text,
									XmNbottomShadowColor,
									&(element_point_viewer->textfield_bottom_shadow_color),NULL);
								XtVaGetValues(element_point_viewer->xi_text,XmNtopShadowColor,
									&(element_point_viewer->textfield_top_shadow_color),NULL);

								XtVaGetValues(element_point_viewer->apply_options_button,
									XmNhighlightColor,
									&(element_point_viewer->label_foreground_color),NULL);
								XtVaGetValues(element_point_viewer->apply_options_button,
									XmNbackground,
									&(element_point_viewer->label_background_color),NULL);

								/* apply options are initially hidden */
								XtUnmanageChild(element_point_viewer->apply_options);
								XtManageChild(element_point_viewer->widget);
								init_widgets=1;
								if (!(element_point_viewer->element_widget=
									CREATE_TEXT_CHOOSE_OBJECT_WIDGET(FE_element)(
										element_point_viewer->element_form,
										element_point_viewer->element_point_identifier.element,
										element_manager,
										(MANAGER_CONDITIONAL_FUNCTION(FE_element) *)NULL,
										(void *)NULL,
										FE_element_to_any_element_string,
										any_element_string_to_FE_element)))
								{
									init_widgets=0;
								}
								if (!(element_point_viewer->top_level_element_widget=
									CREATE_TEXT_CHOOSE_OBJECT_WIDGET(FE_element)(
										element_point_viewer->top_level_element_form,
										element_point_viewer->
										element_point_identifier.top_level_element,
										element_manager,FE_element_is_top_level,(void *)NULL,
										FE_element_to_element_string,
										element_string_to_FE_element)))
								{
									init_widgets=0;
								}
								valid_strings = Xi_discretization_mode_get_valid_strings_for_Element_point_ranges(
									&number_of_valid_strings);
								if (!(element_point_viewer->xi_discretization_mode_widget=
									create_choose_enumerator_widget(
										element_point_viewer->xi_discretization_mode_form,
										number_of_valid_strings,valid_strings,
										ENUMERATOR_STRING(Xi_discretization_mode)(
											element_point_viewer->
											element_point_identifier.xi_discretization_mode))))
								{
									init_widgets=0;
								}
								DEALLOCATE(valid_strings);
								if (!(grid_field=
									FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
										"grid_point_number",computed_field_manager)))
								{
									grid_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
										Computed_field_is_scalar_integer,(void *)NULL,
										computed_field_manager);
								}
								if (!(element_point_viewer->grid_field_widget=
									CREATE_CHOOSE_OBJECT_WIDGET(Computed_field)(
										element_point_viewer->grid_field_form,grid_field,
										computed_field_manager,Computed_field_is_scalar_integer,
										(void *)NULL)))
								{
									init_widgets=0;
								}
								/* pass identifier with copy_element to viewer widget */
								COPY(Element_point_ranges_identifier)(
									&temp_element_point_identifier,
									&(element_point_viewer->element_point_identifier));
								temp_element_point_number=
									element_point_viewer->element_point_number;
								if (temp_element_point_identifier.element)
								{
									Element_point_make_top_level(&temp_element_point_identifier,
										&temp_element_point_number);
								}
								/* pass identifier with copy_element to viewer widget */
								temp_element_point_identifier.element=
									element_point_viewer->element_copy;
								temp_element_point_identifier.top_level_element=
									element_point_viewer->element_copy;
								if (!create_element_point_viewer_widget(
									&(element_point_viewer->viewer_widget),
									element_point_viewer->viewer_form,
									computed_field_package,
									element_point_viewer->modified_field_components,
									&temp_element_point_identifier,
									temp_element_point_number))
								{
									init_widgets=0;
								}
								if (!(element_point_viewer->match_grid_field_widget=
									CREATE_CHOOSE_OBJECT_WIDGET(Computed_field)(
										element_point_viewer->match_grid_field_form,
										element_point_viewer->match_grid_field,
										computed_field_manager,Computed_field_is_scalar_integer,
										(void *)NULL)))
								{
									init_widgets=0;
								}
								if (init_widgets)
								{
									callback.data=(void *)element_point_viewer;
									callback.procedure=Element_point_viewer_update_grid_field;
									CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
										element_point_viewer->grid_field_widget,&callback);
									callback.procedure=
										Element_point_viewer_update_xi_discretization_mode;
									choose_enumerator_set_callback(
										element_point_viewer->xi_discretization_mode_widget,
										&callback);
									callback.procedure=Element_point_viewer_update_element;
									TEXT_CHOOSE_OBJECT_SET_CALLBACK(FE_element)(
										element_point_viewer->element_widget,&callback);
									callback.procedure=
										Element_point_viewer_update_top_level_element;
									TEXT_CHOOSE_OBJECT_SET_CALLBACK(FE_element)(
										element_point_viewer->top_level_element_widget,&callback);
									Element_point_viewer_refresh_chooser_widgets(
										element_point_viewer);
									callback.procedure=
										Element_point_viewer_update_match_grid_field;
									CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
										element_point_viewer->match_grid_field_widget,
										&callback);
									Element_point_viewer_refresh_match_grid_field(
										element_point_viewer);
									element_point_viewer->element_manager_callback_id=
										MANAGER_REGISTER(FE_element)(
											Element_point_viewer_element_change,
											(void *)element_point_viewer,element_manager);
									element_point_viewer->node_manager_callback_id=
										MANAGER_REGISTER(FE_node)(Element_point_viewer_node_change,
											(void *)element_point_viewer,node_manager);
									XtRealizeWidget(element_point_viewer->window_shell);
									XtPopup(element_point_viewer->window_shell,XtGrabNone);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"CREATE(Element_point_viewer).  Could not init widgets");
									DESTROY(Element_point_viewer)(&element_point_viewer);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"CREATE(Element_point_viewer).  "
									"Could not fetch element_point_viewer");
								DESTROY(Element_point_viewer)(&element_point_viewer);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"CREATE(Element_point_viewer).  Could not register identifiers");
							DESTROY(Element_point_viewer)(&element_point_viewer);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"CREATE(Element_point_viewer).  Could not register callbacks");
						DESTROY(Element_point_viewer)(&element_point_viewer);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"CREATE(Element_point_viewer).  Could not create popup shell.");
					DESTROY(Element_point_viewer)(&element_point_viewer);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Element_point_viewer).  Not enough memory");
				if (element_point_viewer)
				{
					DEALLOCATE(element_point_viewer);
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Element_point_viewer).  Could not open hierarchy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Element_point_viewer).  Invalid argument(s)");
	}
	if (element_point_viewer_address)
	{
		*element_point_viewer_address=element_point_viewer;
	}
	LEAVE;

	return (element_point_viewer);
} /* CREATE(Element_point_viewer) */

int DESTROY(Element_point_viewer)(
	struct Element_point_viewer **element_point_viewer_address)
/*******************************************************************************
LAST MODIFIED : 15 June 2000

DESCRIPTION:
Destroys the Element_point_viewer. See also Element_point_viewer_close_CB.
==============================================================================*/
{
	int return_code;
	struct Element_point_viewer *element_point_viewer;

	ENTER(DESTROY(element_point_viewer));
	if (element_point_viewer_address&&
		(element_point_viewer= *element_point_viewer_address))
	{
		DESTROY(LIST(Field_value_index_ranges))(
			&(element_point_viewer->modified_field_components));
		if (element_point_viewer->element_manager_callback_id)
		{
			MANAGER_DEREGISTER(FE_element)(
				element_point_viewer->element_manager_callback_id,
				element_point_viewer->element_manager);
		}
		if (element_point_viewer->node_manager_callback_id)
		{
			MANAGER_DEREGISTER(FE_node)(
				element_point_viewer->node_manager_callback_id,
				element_point_viewer->node_manager);
		}
		/* end callbacks from global element_point selection */
		Element_point_ranges_selection_remove_callback(
			element_point_viewer->element_point_ranges_selection,
			Element_point_viewer_element_point_ranges_selection_change,
			(void *)element_point_viewer);
		/* deaccess the local element_copy */
		REACCESS(FE_element)(&(element_point_viewer->element_copy),
			(struct FE_element *)NULL);
		if (element_point_viewer->window_shell)
		{
			destroy_Shell_list_item_from_shell(&(element_point_viewer->window_shell),
				element_point_viewer->user_interface);
			XtDestroyWidget(element_point_viewer->window_shell);
		}
		DEALLOCATE(*element_point_viewer_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Element_point_viewer).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Element_point_viewer) */

int Element_point_viewer_bring_window_to_front(
	struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 24 May 2000

DESCRIPTION :
Pops the window for <element_point_viewer> to the front of those visible.
==============================================================================*/
{
	int return_code;

	ENTER(Element_point_viewer_bring_window_to_front);
	if (element_point_viewer)
	{
		XtPopup(element_point_viewer->window_shell,XtGrabNone);
		XtVaSetValues(element_point_viewer->window_shell,XmNiconic,False,NULL);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_bring_window_to_front.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_viewer_bring_window_to_front */
