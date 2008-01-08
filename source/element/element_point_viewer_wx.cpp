/*******************************************************************************
FILE : element_point_viewer_wx.cpp

LAST MODIFIED : 6 June 2007

DESCRIPTION :
For wxWidgets only, Dialog for selecting an element point, viewing and editing its fields and
applying changes. Works with Element_point_ranges_selection to display the last
selected element point, or set it if entered in this dialog.
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
extern "C" {
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_value_index_ranges.h"
#include "element/element_point_viewer_wx.h"
#include "finite_element/finite_element_discretization.h"
#include "finite_element/finite_element_to_graphics_object.h"
#include "general/debug.h"
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
}
#if defined (WX_USER_INTERFACE)
#include <wx/collpane.h>
#include "wx/wx.h"
#include "wx/xrc/xmlres.h"
#include "choose/choose_manager_class.hpp"
#include "choose/choose_enumerator_class.hpp" 
#include "element/element_point_viewer_wx.xrch"
#include "choose/text_choose_from_fe_element.hpp"
#include "icon/cmiss_icon.xpm"
#endif /*defined (WX_USER_INTERFACE)*/

/*
Module variables
----------------
*/
#if defined (WX_USER_INTERFACE)
class wxElementPointViewer;
#endif /* defined (WX_USER_INTERFACE) */

struct Element_point_viewer
/*******************************************************************************
LAST MODIFIED : 19 March 2003

DESCRIPTION :
Contains all the information carried by the element_point_viewer widget.
==============================================================================*/
{
	/* global data */
	struct Computed_field_package *computed_field_package;
	struct Element_point_viewer **element_point_viewer_address;
	struct Cmiss_region *region;
	struct FE_region *fe_region;
	struct Element_point_ranges_selection *element_point_ranges_selection;
	struct Time_object *time_object;
	struct User_interface *user_interface;
	/* information about the element point being edited; note element in
		 identifier is not accessed */
	struct Element_point_ranges_identifier element_point_identifier;
	 int element_point_number, number_of_components;
	/* accessed local copy of the element being edited */
	 struct FE_element *element_copy, *template_element;
	struct Computed_field *current_field;
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	/* field components whose values have been modified stored in following */
	struct LIST(Field_value_index_ranges) *modified_field_components;
	/* apply will set same values to all grid points with this field the same,
		 if non-NULL */
	 struct Computed_field *match_grid_field;
	 int time_object_callback;
	struct MANAGER(Computed_field) *computed_field_manager;
#if defined (WX_USER_INTERFACE)
	 wxElementPointViewer *wx_element_point_viewer;
	 wxScrolledWindow *variable_viewer_panel;
	 wxScrolledWindow *collpane;
	 wxWindow *element_point_win;
	 wxBoxSizer *paneSz;
	 wxFrame *frame;
	 wxGridSizer *element_point_grid_field;
	 wxSize frame_size;
	 wxTextCtrl *xitextctrl, *pointtextctrl, *discretizationtextctrl, *gridvaluetextctrl;
	 wxPanel *xi_discretization_mode_chooser_panel;
	 int init_width, init_height, frame_width, frame_height;
	 //  wxBoxSizer *sizer_1;
#endif /* (WX_USER_INTERFACE) */

}; /* element_point_viewer_struct */


/* 
Prototype
----------------
*/
static int element_point_viewer_setup_components(
	 struct Element_point_viewer *element_point_viewer);
/*******************************************************************************
LAST MODIFIED : 10 October 2003

DESCRIPTION :
Creates the array of cells containing field component names and values.
==============================================================================*/

int element_point_viewer_set_element_point_field(
  void *element_point_viewer_void,
	struct Element_point_ranges_identifier *element_point_identifier,
	int element_point_number,struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 6 Jube 2007

DESCRIPTION :
Sets the element_point/field being edited in the
<element_point_field_viewer_widget>.
Note that the viewer works on the element itself, not a local copy. Hence, only
pass unmanaged elements in the element_point_identifier to this widget.
==============================================================================*/

static int Element_point_viewer_refresh_chooser(
	 struct Element_point_viewer *element_point_viewer);
/*******************************************************************************
LAST MODIFIED :7 June 2007

DESCRIPTION :
Updates the element shown in the chooser to match that for the current point.
==============================================================================*/

int element_point_viewer_widget_set_element_point(
	Element_point_viewer *element_point_viewer,
	struct Element_point_ranges_identifier *element_point_identifier,
	int element_point_number);
/*******************************************************************************
LAST MODIFIED : 6 June 2007

DESCRIPTION :
Sets the element point being edited in the <element_point_viewer_widget>. Note
that the viewer works on the element itself, not a local copy. Hence, only pass
unmanaged elements in the identifier to this widget.
==============================================================================*/

static int element_point_viewer_add_collpane(struct Computed_field *current_field,
	 void *element_point_viewer_void);

/*
Module functions
----------------
*/
static int Element_point_viewer_refresh_xi_discretization_mode(
	 struct Element_point_viewer *element_point_viewer);
/*******************************************************************************
LAST MODIFIED : 13 June 2007

DESCRIPTION :
Updates the Xi_discretization_mode shown in the chooser to match that for the
current point.
==============================================================================*/


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
				const_cast<char *>(element_point_viewer->discretizationtextctrl->GetValue().c_str()))
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
					 is_editable=0;
					 is_sensitive=0;
				}
				/* only set string if different from that shown */
				if (strcmp(temp_string,value_string))
				{
					 element_point_viewer->discretizationtextctrl->SetValue(temp_string);
				}
				element_point_viewer->discretizationtextctrl->Enable(is_sensitive);
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
		if (value_string=const_cast<char *>(element_point_viewer->pointtextctrl->GetValue().c_str()))
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
				is_editable=0;
				is_sensitive=0;
			}
			/* only set string if different from that shown */
			if (strcmp(temp_string,value_string))
			{
				 element_point_viewer->pointtextctrl->SetValue(temp_string);
			}
			element_point_viewer->pointtextctrl->Enable(is_sensitive);
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
LAST MODIFIED : 13 June 2007

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
		if (value_string =	const_cast<char *>(element_point_viewer->xitextctrl->GetValue().c_str()))
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
				is_sensitive=1;
			}
			else
			{
				sprintf(temp_string,"N/A");
				is_editable=0;
				is_sensitive=0;
			}
			/* only set string if different from that shown */
			if (strcmp(temp_string,value_string))
			{
				 element_point_viewer->xitextctrl->SetValue(temp_string);
			}
			element_point_viewer->xitextctrl->Enable(is_sensitive);
			element_point_viewer->xitextctrl->SetEditable(is_editable);
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
	 struct Element_point_viewer *element_point_viewer);
/*******************************************************************************
LAST MODIFIED : 13 June 2007

DESCRIPTION :
Updates the grid_value text field. If there is a current element point, writes
the field value, otherwise N/A.
==============================================================================*/

static int Element_point_viewer_apply_changes(struct Element_point_viewer *element_point_viewer, int apply_all)
/*******************************************************************************
LAST MODIFIED : 2 May 2007

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
								return_code = FE_region_for_each_FE_element(
									element_point_viewer->fe_region,
									FE_element_grid_to_Element_point_ranges_list,
									(void *)&grid_to_list_data);
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
							set_grid_values_data.fe_region =
								element_point_viewer->fe_region;
							/* if following flag is cleared it means that some of the selected
								 element points are not grid points */
							set_grid_values_data.number_of_points = 0;
							set_grid_values_data.number_of_points_set = 0;
							FE_region_begin_change(element_point_viewer->fe_region);
							return_code=FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
								Element_point_ranges_set_grid_values,
								(void *)&set_grid_values_data,element_point_ranges_list);
							if (set_grid_values_data.number_of_points != set_grid_values_data.number_of_points_set)
							{
								display_message(WARNING_MESSAGE,
									"Values only set at %d element locations out of %d specified.",
									set_grid_values_data.number_of_points_set, set_grid_values_data.number_of_points);
							}
							FE_region_end_change(element_point_viewer->fe_region);
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
} /* element_point_viewer_apply_changes */

static int element_point_field_is_editable(
	struct Element_point_ranges_identifier *element_point_identifier,
	struct Computed_field *field,int *number_in_xi)
/*******************************************************************************
LAST MODIFIED : 6 June 2000

DESCRIPTION :
Returns true if <field> is editable at <element_point_identifier>, and if so
returns the <number_in_xi>. Returns 0 with no error if no field or no element
is supplied.
==============================================================================*/
{
	int dimension,i,return_code;

	ENTER(element_point_field_is_editable);
	if (element_point_identifier&&number_in_xi)
	{
		if (element_point_identifier->element&&field&&
			(XI_DISCRETIZATION_CELL_CORNERS==
				element_point_identifier->xi_discretization_mode)&&
			Computed_field_get_native_discretization_in_element(field,
				element_point_identifier->element,number_in_xi))
		{
			return_code=1;
			/* check matching discretization */
			dimension=get_FE_element_dimension(element_point_identifier->element);
			for (i=0;(i<dimension)&&return_code;i++)
			{
				if (element_point_identifier->number_in_xi[i] != number_in_xi[i])
				{
					return_code=0;
				}
			}
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_point_field_is_editable.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

static int Element_point_viewer_set_viewer_element_point(
	struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 19 March 2003

DESCRIPTION :
Gets the current element_point, makes a copy of its element if not NULL,
and passes it to the element_point_viewer_widget.
==============================================================================*/
{
	int i,number_of_faces,temp_element_point_number,return_code;
	struct CM_element_information element_identifier;
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
				get_FE_element_identifier(temp_element_point_identifier.element,
					 &element_identifier);
				if (element_point_viewer->element_copy = ACCESS(FE_element)(
							 CREATE(FE_element)(&element_identifier, (struct FE_element_shape *)NULL,
									(struct FE_region *)NULL, temp_element_point_identifier.element)))
				{
					 /* clear the faces of element_copy as messes up exterior calculations
							for graphics created from them */
					 if (get_FE_element_number_of_faces(element_point_viewer->element_copy,
								 &number_of_faces))
					 {
							for (i = 0; i < number_of_faces; i++)
							{
								 set_FE_element_face(element_point_viewer->element_copy,i,
										(struct FE_element *)NULL);
							}
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
				element_point_viewer,
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

static int Element_point_viewer_calculate_xi(
	struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 23 April 2001

DESCRIPTION :
Ensures xi is correct for the currently selected element point, if any.
==============================================================================*/
{
	int return_code;

	ENTER(Element_point_viewer_calculate_xi);
	if (element_point_viewer)
	{
		if (element_point_viewer->element_point_identifier.element)
		{
			return_code = FE_element_get_numbered_xi_point(
				element_point_viewer->element_point_identifier.element,
				element_point_viewer->element_point_identifier.xi_discretization_mode,
				element_point_viewer->element_point_identifier.number_in_xi,
				element_point_viewer->element_point_identifier.exact_xi,
				/*coordinate_field*/(struct Computed_field *)NULL,
				/*density_field*/(struct Computed_field *)NULL,
				element_point_viewer->element_point_number,
				element_point_viewer->xi, Time_object_get_current_time(
				element_point_viewer->time_object));
		}
		else
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_calculate_xi.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_viewer_calculate_xi */

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
			Element_point_viewer_refresh_chooser(element_point_viewer);
			Element_point_viewer_set_viewer_element_point(element_point_viewer);
			if (element_point_viewer->wx_element_point_viewer && element_point_viewer->collpane)
			{
				 FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
						element_point_viewer_add_collpane,
						(void *)element_point_viewer,
						element_point_viewer->computed_field_manager);
			}
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

static void Element_point_viewer_FE_region_change(struct FE_region *fe_region,
	struct FE_region_changes *changes, void *element_point_viewer_void)
/*******************************************************************************
LAST MODIFIED : 6 June 2007

DESCRIPTION :
Callback for changes from the FE_region. If the element currently being viewed
has changed, re-send to viewer.
Note that we do not have to handle add, delete and identifier change messages
here as the select widget does this for us. Only changes to the content of the
object cause updates.
==============================================================================*/
{
	enum CHANGE_LOG_CHANGE(FE_element) fe_element_change;
	enum CHANGE_LOG_CHANGE(FE_node) fe_node_change;
	int i, number_of_nodes, refresh;
	struct Element_point_viewer *element_point_viewer;
	struct FE_element *top_level_element;
	struct FE_node *node;

	ENTER(Element_point_viewer_FE_region_change);
	if (fe_region && changes && (element_point_viewer =
		(struct Element_point_viewer *)element_point_viewer_void))
	{
		if (element_point_viewer->element_point_identifier.element)
		{
			refresh = 0;
			/* check if contents of this element changed */
			if (CHANGE_LOG_QUERY(FE_element)(changes->fe_element_changes,
				element_point_viewer->element_point_identifier.element,
				&fe_element_change) && (fe_element_change &
					CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_element)))
			{
				refresh = 1;
			}
			else
			{
				/* check if nodes in this element have changed */
				if ((top_level_element =
					element_point_viewer->element_point_identifier.top_level_element) &&
					get_FE_element_number_of_nodes(top_level_element, &number_of_nodes) &&
					(0 < number_of_nodes))
				{
					for (i = 0; (i < number_of_nodes) && (!refresh); i++)
					{
						if (get_FE_element_node(top_level_element, i, &node) && node &&
							CHANGE_LOG_QUERY(FE_node)(changes->fe_node_changes, node,
								&fe_node_change) && (fe_node_change &
									CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_node)))
						{
							refresh = 1;
						}
					}
				}
			}
			if (refresh)
			{
				/* update grid_text in case number changed */
				 //				Element_point_viewer_refresh_grid_value_text(element_point_viewer);
				Element_point_viewer_set_viewer_element_point(element_point_viewer);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_FE_region_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_point_viewer_FE_region_change */

static int element_point_viewer_time_change_callback(
	struct Time_object *time_object, double current_time,
	void *element_point_field_viewer_void)
/*******************************************************************************
LAST MODIFIED : 6 June 2007

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Element_point_viewer *element_point_viewer;

	ENTER(element_point_viewer_time_change_callback);
	USE_PARAMETER(current_time);
	if (time_object && (element_point_viewer = 
				(struct Element_point_viewer *)
		element_point_field_viewer_void))
	{
		 //		element_point_field_viewer_widget_update_values(element_point_field_viewer);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_point_viewer_time_change_callback.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	return(return_code);
} /* element_point_viewer_time_change_callback */	

class wxElementPointViewer : public wxFrame
{	
	 Element_point_viewer *element_point_viewer;
	 wxPanel *node_text_panel, *element_text_panel, *top_element_text_panel,
			*grid_field_chooser_panel;
	 // 	 wxScrolledWindow *variable_viewer_panel;
	 wxFeElementTextChooser *element_text_chooser;
	 wxFeElementTextChooser *top_level_element_text_chooser;
	 wxFrame *frame;
	 wxTextCtrl *discretizationtextctrl, *pointtextctrl;
	 DEFINE_ENUMERATOR_TYPE_CLASS(Xi_discretization_mode);
	 Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Xi_discretization_mode)>
	 *xi_discretization_mode_chooser;
	 DEFINE_MANAGER_CLASS(Computed_field);
	 Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
		*grid_field_chooser;
public:
	 
	 wxElementPointViewer(Element_point_viewer *element_point_viewer): 
			element_point_viewer(element_point_viewer)
	 {
			wxXmlInit_element_point_viewer_wx();
			element_point_viewer->wx_element_point_viewer = (wxElementPointViewer *)NULL;
			wxXmlResource::Get()->LoadFrame(this,
				 (wxWindow *)NULL, _T("CmguiElementPointViewer"));
			this->SetIcon(cmiss_icon_xpm);

			element_text_panel =
				 XRCCTRL(*this, "ElementTextPanel",wxPanel);
			element_text_chooser =
				 new wxFeElementTextChooser(element_text_panel,
						element_point_viewer->element_point_identifier.element, 
						element_point_viewer->fe_region, 
						(LIST_CONDITIONAL_FUNCTION(FE_element) *)NULL,(void *)NULL); 
			Callback_base<FE_element *> *element_text_callback = 
				 new Callback_member_callback< FE_element*, 
				 wxElementPointViewer, int (wxElementPointViewer::*)(FE_element *) >
				 (this, &wxElementPointViewer::element_text_callback);
			element_text_chooser->set_callback(element_text_callback);

			top_element_text_panel =
				 XRCCTRL(*this, "TopElementTextPanel",wxPanel);
			top_level_element_text_chooser =
				 new wxFeElementTextChooser(top_element_text_panel,
						element_point_viewer->element_point_identifier.top_level_element, 
						element_point_viewer->fe_region, 
						FE_element_is_top_level,(void *)NULL); 
			Callback_base<FE_element *> *top_element_text_callback = 
				 new Callback_member_callback< FE_element*, 
				 wxElementPointViewer, int (wxElementPointViewer::*)(FE_element *) >
				 (this, &wxElementPointViewer::top_element_text_callback);
			top_level_element_text_chooser->set_callback(top_element_text_callback);

			element_point_viewer->xi_discretization_mode_chooser_panel=XRCCTRL(
				 *this,"XiDiscretizationModePanel",wxPanel);
			xi_discretization_mode_chooser = new Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Xi_discretization_mode)>
				 (element_point_viewer->xi_discretization_mode_chooser_panel, 
						element_point_viewer->element_point_identifier.xi_discretization_mode,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Xi_discretization_mode) *)NULL,
						(void *)NULL, element_point_viewer->user_interface);
			element_point_viewer->xi_discretization_mode_chooser_panel->Fit();
			Callback_base< enum Xi_discretization_mode > *xi_discretization_mode_callback = 
				 new Callback_member_callback<enum Xi_discretization_mode, 
				 wxElementPointViewer, int(wxElementPointViewer::*)(enum Xi_discretization_mode)>
				 (this, &wxElementPointViewer::xi_discretization_mode_callback);
			xi_discretization_mode_chooser->set_callback(xi_discretization_mode_callback);
			if (element_point_viewer->element_point_identifier.element)
			{
				 element_point_viewer->xi_discretization_mode_chooser_panel->Enable(1);
			}
			else
			{
				 element_point_viewer->xi_discretization_mode_chooser_panel->Enable(0);
			}

			element_point_viewer->discretizationtextctrl = XRCCTRL(*this,"DiscretizationTextCtrl",wxTextCtrl);
			Element_point_viewer_refresh_discretization_text(element_point_viewer);
			element_point_viewer->pointtextctrl = XRCCTRL(*this, "PointTextCtrl", wxTextCtrl);
			Element_point_viewer_refresh_point_number_text(element_point_viewer);
			element_point_viewer->xitextctrl = 
				 XRCCTRL(*this,"XiTextCtrl", wxTextCtrl);
			Element_point_viewer_refresh_xi_text(element_point_viewer);

			struct Computed_field *grid_field;
			struct MANAGER(Computed_field) *computed_field_manager;
			if (computed_field_manager = Computed_field_package_get_computed_field_manager(
						 element_point_viewer->computed_field_package))
			{
				 if (!(grid_field=
							 FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
									"grid_point_number",computed_field_manager)))
				 {
						grid_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
							 Computed_field_is_scalar_integer,(void *)NULL,
							 computed_field_manager);
				 }
			}
			grid_field_chooser_panel=XRCCTRL(
				 *this,"GridFieldPanel",wxPanel);
			grid_field_chooser =
				 new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
				 (grid_field_chooser_panel, grid_field, computed_field_manager,
						Computed_field_is_scalar_integer, (void *)NULL, element_point_viewer->user_interface);
			Callback_base< Computed_field* > *grid_field_callback = 
				 new Callback_member_callback< Computed_field*, 
				 wxElementPointViewer, int (wxElementPointViewer::*)(Computed_field *) >
				 (this, &wxElementPointViewer::grid_field_callback);
			grid_field_chooser->set_callback(grid_field_callback);

			element_point_viewer->gridvaluetextctrl=XRCCTRL(*this, "GridValueTextCtrl", wxTextCtrl);
			Show();
			frame = XRCCTRL(*this, "CmguiElementPointViewer",wxFrame);
			frame->GetSize(&(element_point_viewer->init_width), &(element_point_viewer->init_height));
			frame->SetSize(element_point_viewer->frame_width,element_point_viewer->frame_height+100);
			frame->GetSize(&(element_point_viewer->frame_width), &(element_point_viewer->frame_height));
			frame->Layout();		
	 };
	 
	 wxElementPointViewer()
	 {
	 };

  ~wxElementPointViewer()
	 {
			delete element_text_chooser;
			delete top_level_element_text_chooser;
			delete xi_discretization_mode_chooser;
			delete grid_field_chooser;
	 }

int element_text_callback(FE_element *element)
/*******************************************************************************
LAST MODIFIED :7 June 2007

DESCRIPTION :
Callback from wxTextChooser when text is entered.
==============================================================================*/
{
	 FE_value element_to_top_level[9];
	 
	 if (element_point_viewer)
	 {
			/* don't select elements until there is a top_level_element */
			if (element_point_viewer->element_point_identifier.top_level_element&&
				 (element))
			{
				 element_point_viewer->element_point_identifier.element=element;
				 if (element)
				 {
						/* get top_level_element, keeping existing one if possible */
						element_point_viewer->element_point_identifier.top_level_element=
							 FE_element_get_top_level_element_conversion(
									element_point_viewer->element_point_identifier.element,
									element_point_viewer->element_point_identifier.top_level_element,
									(struct LIST(FE_element) *)NULL,/*face_number*/-1,
									element_to_top_level);
						top_level_element_text_chooser->set_object(
							 element_point_viewer->element_point_identifier.top_level_element);
						//				top_element_text_callback(
						// 					 element_point_viewer->element_point_identifier.top_level_element);
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
	 return 1;
}
	 
int top_element_text_callback(FE_element *top_level_element)
/*******************************************************************************
LAST MODIFIED :7 June 2007

DESCRIPTION :
Callback from wxTextChooser when text is entered.
==============================================================================*/
{
	 int i;

	 if (element_point_viewer)
	 {
			if (top_level_element)
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
							 element_point_viewer->element_point_identifier.top_level_element =
									FE_region_get_first_FE_element_that(element_point_viewer->fe_region,
										 FE_element_is_top_level_parent_of_element,
										 (void *)element_point_viewer->element_point_identifier.element);
							 top_level_element_text_chooser->set_object(
									element_point_viewer->element_point_identifier.top_level_element);
						}
				 }
				 else
				 {
						/* use the top_level_element for the element too */
						element_point_viewer->element_point_identifier.element=
							 top_level_element;
						element_text_chooser->set_object(
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
	 
			return 1;
}

int xi_discretization_mode_callback(enum Xi_discretization_mode xi_discretization_mode)
/*******************************************************************************
LAST MODIFIED : 22 March 2007

DESCRIPTION :
Callback from wxChooser<xi Discretization Mode> when choice is made.
==============================================================================*/
{
	 int i, is_editable;
	struct Element_point_ranges_identifier temp_element_point_identifier;
	is_editable = 0;
	if (element_point_viewer)
	{
		/* store old identifier unless in case new one is invalid */
		COPY(Element_point_ranges_identifier)(&temp_element_point_identifier,
			&(element_point_viewer->element_point_identifier));
		if (xi_discretization_mode)
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
					is_editable = 1;
				}
			}
			if (element_point_viewer->xitextctrl)
			{
				 element_point_viewer->xitextctrl->SetEditable(is_editable);
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
	 return 1;
}

int grid_field_callback(Computed_field *grid_field)
/*******************************************************************************
LAST MODIFIED :7 June 2007

DESCRIPTION :
Callback from wxTextChooser when text is entered.
==============================================================================*/
{
	 if (element_point_viewer)
	 {
			Element_point_viewer_refresh_grid_value_text(element_point_viewer);
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "Element_point_viewer_update_grid_field.  Invalid argument(s)");
	 }
	 
	 return 1;
}

void RenewElementPointViewer(wxCollapsiblePaneEvent& event)
/*******************************************************************************
LAST MODIFIED : 14 June 2007

DESCRIPTION :
==============================================================================*/
 {
		wxScrolledWindow *VariableViewer = XRCCTRL(
			 *this,"ElementVariableViewerPanel",wxScrolledWindow);
		VariableViewer->Layout();	
		frame = XRCCTRL(*this, "CmguiElementPointViewer", wxFrame);
		frame->SetMinSize(wxSize(50,100));
		frame->SetMaxSize(wxSize(2000,2000));
 }

void OnPointValueEntered(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 14 June 2007

DESCRIPTION :
Called when entry is made into the point_number_text field.
==============================================================================*/
{
	 char *value_string;
	 int element_point_number;
	 
	 if (element_point_viewer)
	 {
			/* Get the text string */
			if (value_string=
				 const_cast<char *>(element_point_viewer->pointtextctrl->GetValue().c_str()))
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
				 
			}
			else
			{
				 display_message(ERROR_MESSAGE,
						"Element_point_viewer_point_number_text_CB.  Missing text");
			}
			/* always restore point_number_text to actual value stored */
			Element_point_viewer_refresh_point_number_text(element_point_viewer);
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "Element_point_viewer_point_number_text_CB.  Invalid argument(s)");
	 }
} /* Element_point_viewer_point_number_text_CB */

void OnDiscretizationValueEntered(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 14 June 2007

DESCRIPTION :
Called when entry is made into the discretization_text field.
==============================================================================*/
{
	char *value_string;
	struct Element_discretization discretization,temp_discretization;
	struct Parse_state *temp_state;

	if (element_point_viewer)
	{
		 /* Get the text string */
		 if (value_string=
				const_cast<char *>(element_point_viewer->discretizationtextctrl->GetValue().c_str()))
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
		 }
		 else
		 {
				display_message(ERROR_MESSAGE,
					 "Element_point_viewer_discretization_text_CB.  Missing text");
		 }
		 
		 /* always restore discretization_text to actual value stored */
		 Element_point_viewer_refresh_discretization_text(element_point_viewer);
	}
	else
	{
		 display_message(ERROR_MESSAGE,
				"Element_point_viewer_discretization_text_CB.  Invalid argument(s)");
	}
} /* Element_point_viewer_discretization_text_CB */

void OnXiValueEntered(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 14 June 2007

DESCRIPTION :
Called when entry is made into the xi_text field.
==============================================================================*/
{
	char *value_string;
	float xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int dimension,i;
	struct FE_element *element;
	struct Parse_state *temp_state;

	if (element_point_viewer)
	{
		 if ((element = element_point_viewer->element_point_identifier.element) &&
				(dimension = get_FE_element_dimension(element)))
		 {
				/* Get the text string */
				if (value_string=
					 const_cast<char *>(element_point_viewer->xitextctrl->GetValue().c_str()))
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
				}
				else
				{
					 display_message(ERROR_MESSAGE,
							"Element_point_viewer_xi_text_CB.  Missing text");
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
} /* Element_point_viewer_xi_text_CB */

void OnGridValueEntered(wxCommandEvent &event)
{
	 char *value_string;
	 int grid_value;
	 struct Computed_field *grid_field;
	 struct FE_element_grid_to_Element_point_ranges_list_data grid_to_list_data;
	 struct FE_field *grid_fe_field;
	 if (element_point_viewer)
	 {
			/* Get the text string */
			if (value_string= const_cast<char *>(element_point_viewer->gridvaluetextctrl->GetValue().c_str()))
			{
				 if ((grid_field=get_grid_field())&&
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
										 /* inefficient: go through every element in FE_region */
										 FE_region_for_each_FE_element(element_point_viewer->fe_region,
												FE_element_grid_to_Element_point_ranges_list,
												(void *)&grid_to_list_data);
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
			}
			else
			{
				 display_message(ERROR_MESSAGE,
						"Element_point_viewer_grid_value_text_CB.  Missing text");
			}
			/* always restore grid_value_text to actual value stored */
			Element_point_viewer_refresh_grid_value_text(element_point_viewer);
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "Element_point_viewer_grid_value_text_CB.  Invalid argument(s)");
	 }
}

void OnApplypressed(wxCommandEvent &event)
{
	 Element_point_viewer_apply_changes(element_point_viewer,/*apply_all*/0);
}

void OnApplyAllpressed(wxCommandEvent &event)
{
	 Element_point_viewer_apply_changes(element_point_viewer,/*apply_all*/1);
}

void OnRevertpressed(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 15 June 2007

DESCRIPTION :
Callback for Revert button. Sends global element point values back to the
editor widget, undoing any modifications.
==============================================================================*/
{
	 if (element_point_viewer)
	 {
			Element_point_viewer_set_viewer_element_point(element_point_viewer);
			if (element_point_viewer->wx_element_point_viewer && element_point_viewer->collpane)
			{
				 FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
						element_point_viewer_add_collpane,
						(void *)element_point_viewer,
						element_point_viewer->computed_field_manager);
			}
	 }
}

void OnCancelpressed(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 16 June 2007

DESCRIPTION :
Callback from the Close button.
Also called when "close" is selected from the window menu.
==============================================================================*/
{
	 if (element_point_viewer)
	{
		DESTROY(Element_point_viewer)(
			element_point_viewer->element_point_viewer_address);
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"Element_point_viewer_close_CB.  Invalid argument(s)");
	}
} /* Element_point_viewer_close_CB */


void FrameSetSize(wxSizeEvent &event)
/*******************************************************************************
LAST MODIFIED : 19 June 2007

DESCRIPTION :
Callback of size event to prevent minising the size of the windows
after a collapsible pane is opened/closed.
==============================================================================*/
{
	 int temp_width, temp_height;
	 
	 frame = XRCCTRL(*this, "CmguiElementPointViewer",wxFrame);
	 frame->Freeze();
	 frame->GetSize(&temp_width, &temp_height);
	 if (temp_height !=element_point_viewer->frame_height || temp_width !=element_point_viewer->frame_width)
	 {
			if (temp_width>element_point_viewer->init_width || temp_height>element_point_viewer->init_height)
			{
				 element_point_viewer->frame_width = temp_width;
				 element_point_viewer->frame_height = temp_height;
			}
			else
			{
				 frame->SetSize(element_point_viewer->frame_width,element_point_viewer->frame_height);
			}
	 }
	 frame->Thaw();
	 frame->Layout();		
}

void Terminate(wxCloseEvent& event)
{
	 if (element_point_viewer)
	 {
			DESTROY(Element_point_viewer)(
				 element_point_viewer->element_point_viewer_address);
	 }
} 

void ElementPointViewerTextEntered(wxTextCtrl *textctrl ,
	 Element_point_viewer *element_point_field_viewer, Computed_field *field, int component_number)
/*******************************************************************************
LAST MODIFIED : 14 June 2007

DESCRIPTION :
Called when the user has changed the data in the text widget.  Processes the
data, and then changes the correct value in the array structure.
==============================================================================*/
{
	 char *field_value_string,*value_string;
	 FE_value time,value,*values,*xi;
	 int dimension,element_point_number,i,int_value,*int_values,
			number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS],
			number_of_grid_values,return_code;
	 struct FE_element *element,*top_level_element;
	 struct FE_field *fe_field;

	 if ((element_point_field_viewer) &&
			(element=element_point_field_viewer->element_point_identifier.element)&&
			(top_level_element=
				 element_point_field_viewer->element_point_identifier.top_level_element)&&
			(xi=element_point_field_viewer->xi)&&
			(field)&&
			(0<=component_number)&&
			(element_point_viewer->current_field=field) &&
			(component_number<Computed_field_get_number_of_components(field)))
	 {
			time = Time_object_get_current_time(element_point_field_viewer->time_object);
			/* get old_value_string to prevent needless updating and preserve text
				 selections for cut-and-paste */
			if (value_string=const_cast<char *>((textctrl->GetValue()).c_str()))
			{
				 if ((XI_DISCRETIZATION_CELL_CORNERS==element_point_field_viewer->
							 element_point_identifier.xi_discretization_mode)&&
						Computed_field_get_native_discretization_in_element(field,element,
							 number_in_xi))
				 {
						/* get the number of values that are stored in the grid */
						dimension=get_FE_element_dimension(element);
						number_of_grid_values=1;
						for (i=0;i<dimension;i++)
						{
							 number_of_grid_values *= (number_in_xi[i]+1);
						}
						element_point_number=element_point_field_viewer->element_point_number;
						/* check the element_point_number is valid */
						if ((0<=element_point_number)&&
							 (element_point_number<number_of_grid_values))
						{
							 return_code=0;
							 if (Computed_field_is_type_finite_element(field)&&
									Computed_field_get_type_finite_element(field,&fe_field)&&
									(INT_VALUE==get_FE_field_value_type(fe_field)))
							 {
									/* handle integer value_type separately to avoid inaccuracies of
										 real->integer conversion */
									if (1==sscanf(value_string,"%d",&int_value))
									{
										 if (get_FE_element_field_component_grid_int_values(element,
													 fe_field,component_number,&int_values))
										 {
												/* change the value for this component */
												int_values[element_point_number]=int_value;
												return_code=set_FE_element_field_component_grid_int_values(
													 element,fe_field,component_number,int_values);
												DEALLOCATE(int_values);
										 }
									}
							 }
							 else
							 {
									if (1==sscanf(value_string,"%g",&value))
									{
										 if (ALLOCATE(values, FE_value, 
													 Computed_field_get_number_of_components(field)))
										 {
												if (Computed_field_evaluate_in_element(
															 field, element, xi, time, 
															 /*top_level*/(struct FE_element *)NULL,
															 values, /*derivative*/(FE_value *)NULL))
												{
													 values[component_number] = value;
													 return_code=Computed_field_set_values_in_element(
															field, element, xi, time, values);
												}
												else
												{
													 display_message(ERROR_MESSAGE,
															"element_point_field_viewer_value_CB.  "
															"Unable to evaluate field component values");
												}
												DEALLOCATE(values);
										 }
										 else
										 {
												display_message(ERROR_MESSAGE,
													 "element_point_field_viewer_value_CB.  "
													 "Unable to allocate temporary storage.");
										 }
										 Computed_field_clear_cache(field);
									}
							 }
							 if (return_code)
							 {
									/* add this field component to the modified list */
									Field_value_index_ranges_list_add_field_value_index(
										 element_point_field_viewer->modified_field_components,
										 field,component_number);
							 }
						}
						else
						{
							 display_message(ERROR_MESSAGE,
									"element_point_viewer_value_CB.  "
									"element_point_number out of range");
						}
				 }
				 else
				 {
						display_message(ERROR_MESSAGE,"element_point_field_viewer_value_CB.  "
							 "Cannot set values for non-grid-based field");
				 }
			}
			/* redisplay the actual value for the field component */
			if (field_value_string=
				 Computed_field_evaluate_as_string_in_element(
						field,component_number,element,xi,time,top_level_element))
			{
				 /* only set string from field if different from that shown */
				 if (strcmp(field_value_string,value_string))
				 {
						textctrl->SetValue(field_value_string);
				 }
				 DEALLOCATE(field_value_string);
			}
			else
			{
				 display_message(ERROR_MESSAGE,
						"element_point_viewer_value_CB.  "
						"Could not get component as string");
			}
			Computed_field_clear_cache(field);
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "element_point_viewer_value_CB.  Invalid argument(s)");
	 }

} /* element_point_field_viewer_value_CB */

int set_element_selection(Element_point_viewer *element_point_viewer)
{
	 return element_text_chooser->set_object(element_point_viewer->element_point_identifier.element);
}

int set_top_level_element_selection(Element_point_viewer *element_point_viewer)
{
	 return top_level_element_text_chooser->set_object(
			element_point_viewer->element_point_identifier.top_level_element);
}
	 
struct Computed_field *get_grid_field()
{
	 Computed_field *grid_field;
	 if (grid_field_chooser)
	 {
			grid_field = grid_field_chooser->get_object();
	 }
	 else
	 {
			grid_field = (Computed_field *)NULL;
	 }
	 return (grid_field);
}
	 
 void set_xi_discretization_mode_chooser_value(Element_point_viewer *element_point_viewer)
{
	 if (xi_discretization_mode_chooser)
	 {
			xi_discretization_mode_chooser->set_value(element_point_viewer->
				 element_point_identifier.xi_discretization_mode);
	 }
}

  DECLARE_DYNAMIC_CLASS(wxElementPointViewer);
  DECLARE_EVENT_TABLE();
};

IMPLEMENT_DYNAMIC_CLASS(wxElementPointViewer, wxFrame)
BEGIN_EVENT_TABLE(wxElementPointViewer, wxFrame)
 	 EVT_COLLAPSIBLEPANE_CHANGED(wxID_ANY,wxElementPointViewer::RenewElementPointViewer)
	 EVT_TEXT_ENTER(XRCID("DiscretizationTextCtrl"),wxElementPointViewer::OnDiscretizationValueEntered)
	 EVT_TEXT_ENTER(XRCID("PointTextCtrl"),wxElementPointViewer::OnPointValueEntered)
	 EVT_TEXT_ENTER(XRCID("XiTextCtrl"),wxElementPointViewer::OnXiValueEntered)
	 EVT_TEXT_ENTER(XRCID("GridValueTextCtrl"),wxElementPointViewer::OnGridValueEntered)
	 EVT_BUTTON(XRCID("ApplyButton"),wxElementPointViewer::OnApplypressed)
	 EVT_BUTTON(XRCID("ApplyAllButton"),wxElementPointViewer::OnApplyAllpressed)
 	 EVT_BUTTON(XRCID("RevertButton"),wxElementPointViewer::OnRevertpressed)
 	 EVT_BUTTON(XRCID("CancelButton"),wxElementPointViewer::OnCancelpressed)
#if !defined (__WXGTK__)
	 EVT_SIZE(wxElementPointViewer::FrameSetSize)
#endif /*!defined (__WXGTK__)*/
	 EVT_CLOSE(wxElementPointViewer::Terminate)
END_EVENT_TABLE()

class wxElementPointViewerTextCtrl : public wxTextCtrl
{
	 Element_point_viewer *element_point_viewer;
	 struct Computed_field *field;
	 int component_number;

public:

  wxElementPointViewerTextCtrl(Element_point_viewer *element_point_viewer, 
		 Computed_field *field,
		 int component_number) :
		 element_point_viewer(element_point_viewer), field(field), 
		 component_number(component_number)
  {
  }

  ~wxElementPointViewerTextCtrl()
  {
  }

  void OnElementPointViewerTextCtrlEntered(wxCommandEvent& Event)
  {
		 element_point_viewer->wx_element_point_viewer->ElementPointViewerTextEntered
				(this, element_point_viewer, field, component_number);
	}

};

char * element_point_viewer_update_value(struct Element_point_viewer *element_point_viewer, 
	 Computed_field *field, int component_number)
/*******************************************************************************
LAST MODIFIED : 30 June 2000

DESCRIPTION :
Updates the value on the text ctrl.
==============================================================================*/
{
	char *value_string;
	FE_value time, *xi;
	int number_of_components;
	struct FE_element *element,*top_level_element;
	struct Field_value_index_ranges *modified_ranges;
	
	ENTER(element_point_viewer_update_value);
	if (element_point_viewer&&(element=element_point_viewer->element_point_identifier.element)&&
		(top_level_element=
			element_point_viewer->element_point_identifier.top_level_element)&&
		(xi=element_point_viewer->xi))
	{
		modified_ranges=FIND_BY_IDENTIFIER_IN_LIST(Field_value_index_ranges,field)(
			field,element_point_viewer->modified_field_components);
		number_of_components=Computed_field_get_number_of_components(field);
		time = Time_object_get_current_time(element_point_viewer->time_object);
		if (!(value_string=Computed_field_evaluate_as_string_in_element(
						 field,component_number,element,xi,time,top_level_element)))
		{
			 value_string = NULL;
			 display_message(ERROR_MESSAGE,
					"element_point_viewer_update_value.  "
					"Could not get component as string");
		}
		Computed_field_clear_cache(field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_point_viewer_update_values.  Invalid argument(s)");
	}
	return(value_string);
	LEAVE;
} /* element_point_field_viewer_widget_update_values */

int element_point_viewer_add_textctrl(int editable, struct Element_point_viewer *element_point_viewer, 
	 Computed_field *field, int component_number)
/*******************************************************************************
LAST MODIFIED : 12 June 2007

DESCRIPTION :
Add textctrl box onto the viewer.
==============================================================================*/
{
	 char *temp_string;
	 wxElementPointViewerTextCtrl *element_point_viewer_text = 
			new wxElementPointViewerTextCtrl(element_point_viewer, field, component_number);
	 if (temp_string = element_point_viewer_update_value(
					element_point_viewer, field, component_number))
	 {
			if (editable)
			{
				 element_point_viewer_text ->Create(element_point_viewer->element_point_win, 
						-1, temp_string, wxPoint(100,component_number * 35),wxDefaultSize,wxTE_PROCESS_ENTER);
			}
			else
			{
				 element_point_viewer_text ->Create (element_point_viewer->element_point_win, -1, 
						temp_string ,wxPoint(100,component_number * 35),wxDefaultSize,wxTE_READONLY);
				 element_point_viewer_text->SetBackgroundColour(wxColour(231, 231, 231, 255));
			}
			DEALLOCATE(temp_string);
	 }
	 else
	 {
			element_point_viewer_text->Create (element_point_viewer->element_point_win, -1, "ERROR"
				 , wxPoint(100,component_number * 35), wxDefaultSize,wxTE_PROCESS_ENTER);	
	 }
	 if (editable)
	 {
			element_point_viewer_text->Connect(wxEVT_COMMAND_TEXT_ENTER,
				 wxCommandEventHandler(wxElementPointViewerTextCtrl::
						OnElementPointViewerTextCtrlEntered));
			element_point_viewer_text->Connect(wxEVT_KILL_FOCUS,
				 wxCommandEventHandler(wxElementPointViewerTextCtrl::OnElementPointViewerTextCtrlEntered));
	 }
	 element_point_viewer->element_point_grid_field->Add(element_point_viewer_text, 0,
			wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxADJUST_MINSIZE, 0); 
	 return (1);
}				

static int element_point_viewer_add_collpane(struct Computed_field *current_field,
	 void *element_point_viewer_void)
{
	 char *field_name;
	struct Element_point_ranges_identifier temp_element_point_identifier;
	int temp_element_point_number;
	 Element_point_viewer *element_point_viewer = (struct Element_point_viewer *)element_point_viewer_void;
	 field_name = (char *)NULL;
	 GET_NAME(Computed_field)(current_field, &field_name);
	 element_point_viewer->current_field = current_field;
	 int condition;
	 if (element_point_viewer->element_point_identifier.element)
	 {
				 condition = Computed_field_is_defined_in_element_conditional(
						current_field, (void *)element_point_viewer->element_point_identifier.element);
	 }
	 else
	 {
			condition = 0;
	 }
	 if (condition)
	 {
			wxScrolledWindow *panel = element_point_viewer->collpane;
			char *identifier;
			int length;
			identifier = (char *)NULL;
			length = 	strlen(field_name);
			COPY(Element_point_ranges_identifier)(
				 &temp_element_point_identifier,
				 &(element_point_viewer->element_point_identifier));
			temp_element_point_number=
				 element_point_viewer->element_point_number;
			/* Check the collapsible panel is created, if yes, renew the
				 content, if collapsible panel does not exist, create a new one */
			if (ALLOCATE(identifier,char,length+length+length+2))
			{
				 strcpy(identifier, field_name);
				 strcat(identifier, field_name);
				 strcat(identifier, field_name);
				 identifier[length+length+length+1]='\0';
			}
			if (element_point_viewer->element_point_win = panel->FindWindowByName(identifier))
			{
				 element_point_viewer->element_point_win->DestroyChildren();
				 element_point_viewer->paneSz = new wxBoxSizer(wxVERTICAL);
			}
			else
			{
				 wxCollapsiblePane *collapsiblepane = new wxCollapsiblePane;
				 wxSizer *sizer = panel->GetSizer();
				 collapsiblepane->Create(panel, /*id*/-1, field_name);
				 sizer->Add(collapsiblepane, 0,wxALL, 5);
				 element_point_viewer->element_point_win = collapsiblepane->GetPane();
				 element_point_viewer->element_point_win->SetName(identifier);
				 element_point_viewer->paneSz = new wxBoxSizer(wxVERTICAL);
			}
			if (identifier)
			{
				 DEALLOCATE(identifier);
			}
 			if (element_point_viewer_set_element_point_field((void *)element_point_viewer,
						&temp_element_point_identifier,
						temp_element_point_number, element_point_viewer->current_field))
			{
				 if (element_point_viewer->element_point_grid_field != NULL)
				 {
						element_point_viewer->element_point_win->SetSizer(
							 element_point_viewer->element_point_grid_field);
						element_point_viewer->element_point_grid_field->SetSizeHints(
							 element_point_viewer->element_point_win);
						element_point_viewer->element_point_grid_field->Layout();
						element_point_viewer->element_point_win->Layout();
				 }
			}
			element_point_viewer->collpane->Layout();
			panel->FitInside();
			panel->SetScrollbars(20, 20, 50, 50);
			panel->Layout();
			wxFrame *frame;
			frame = 
				 XRCCTRL(*element_point_viewer->wx_element_point_viewer, "CmguiElementPointViewer", wxFrame);
			frame->Layout();
			frame->SetMinSize(wxSize(50,100));
	 }
	 if (field_name)
	 {
			DEALLOCATE(field_name);
	 }
	 return 1;
}

/*
Global functions
----------------
*/
struct Element_point_viewer *CREATE(Element_point_viewer)(
	struct Element_point_viewer **element_point_viewer_address,
	struct Cmiss_region *region,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct Computed_field_package *computed_field_package,
	struct Time_object *time_object,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 6 June 2007

DESCRIPTION :
Creates a dialog for choosing element points and displaying and editing their
fields.
==============================================================================*/
{
	 int i,number_of_faces, start, stop, temp_element_point_number;
	 /*temp_element_point_number*/
	struct MANAGER(Computed_field) *computed_field_manager;
// 	MANAGER_CONDITIONAL_FUNCTION(Computed_field)
// 		 *choose_field_conditional_function;
	struct CM_element_information element_identifier;
	//	struct Computed_field *grid_field;
	struct Element_point_ranges *element_point_ranges;
	struct Element_point_ranges_identifier temp_element_point_identifier;
	struct Element_point_viewer *element_point_viewer;
	struct FE_region *fe_region;
	struct Multi_range *ranges;
	struct FE_element *initial_element;

	ENTER(CREATE(Element_point_viewer));
	element_point_viewer=(struct Element_point_viewer *)NULL;
	if (element_point_viewer_address && region &&
		(fe_region = Cmiss_region_get_FE_region(region)) &&
		element_point_ranges_selection&&computed_field_package &&
		(computed_field_manager = Computed_field_package_get_computed_field_manager(
			computed_field_package)) && user_interface)
	{
		 /* allocate memory */
		 if (ALLOCATE(element_point_viewer,struct Element_point_viewer,1)&&
				(element_point_viewer->modified_field_components=
					 CREATE(LIST(Field_value_index_ranges))()))
		 {
				element_point_viewer->computed_field_manager=computed_field_manager;
				element_point_viewer->computed_field_package=computed_field_package;
				element_point_viewer->element_point_viewer_address=
					 element_point_viewer_address;
				element_point_viewer->region = region;
				element_point_viewer->fe_region = fe_region;
				element_point_viewer->element_point_ranges_selection=
					 element_point_ranges_selection;
				element_point_viewer->user_interface=user_interface;
				element_point_viewer->time_object = ACCESS(Time_object)(
					 time_object);
				element_point_viewer->element_point_identifier.element=
					 (struct FE_element *)NULL;
				element_point_viewer->element_point_identifier.top_level_element=
					 (struct FE_element *)NULL;
				element_point_viewer->element_copy=(struct FE_element *)NULL;
				element_point_viewer->element_point_identifier.xi_discretization_mode=
					 XI_DISCRETIZATION_EXACT_XI;
				//				element_point_viewer->time_object_callback = 0;
				element_point_viewer->number_of_components=-1;
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
				element_point_viewer->current_field=(struct Computed_field *)NULL;
				initial_element = (struct FE_element *)NULL;
				/* initialise widgets */
#if defined (WX_USER_INTERFACE)
				element_point_viewer->wx_element_point_viewer = (wxElementPointViewer *)NULL;
				element_point_viewer->element_point_grid_field = NULL;			
#endif /* defined (WX_USER_INTERFACE) */		
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
					 if (element_point_viewer->element_point_identifier.element =
							FE_region_get_first_FE_element_that(fe_region,
								 FE_element_is_top_level, (void *)NULL))
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
					 if (get_FE_element_identifier(
									element_point_viewer->element_point_identifier.top_level_element,
									&element_identifier) &&
							(element_point_viewer->element_copy = ACCESS(FE_element)(
									CREATE(FE_element)(&element_identifier,
										 (struct FE_element_shape *)NULL, (struct FE_region *)NULL,
										 element_point_viewer->
										 element_point_identifier.top_level_element))))
					 {
							/* clear the faces of element_copy as messes up exterior
								 calculations for graphics created from them */
							if (get_FE_element_number_of_faces(
										 element_point_viewer->element_copy, &number_of_faces))
							{
								 for (i=0;i<number_of_faces;i++)
								 {
										set_FE_element_face(element_point_viewer->element_copy,i,
											 (struct FE_element *)NULL);
								 }
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
				/* new code */
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
				initial_element=temp_element_point_identifier.element;
				if (initial_element)
				{
					 element_point_viewer->template_element = ACCESS(FE_element)(
							CREATE(FE_element)(&element_identifier,
								 (struct FE_element_shape *)NULL, (struct FE_region *)NULL,
								 initial_element));
				}
				/* make the dialog shell */
#if defined (WX_USER_INTERFACE)
				element_point_viewer->frame_width = 0;
				element_point_viewer->frame_height = 0;
				element_point_viewer->init_width = 0;
				element_point_viewer->init_height=0;
				wxLogNull logNo;
				element_point_viewer->wx_element_point_viewer = new wxElementPointViewer(element_point_viewer);
				Element_point_viewer_refresh_grid_value_text(element_point_viewer);	
				element_point_viewer->collpane =
					 XRCCTRL(*element_point_viewer->wx_element_point_viewer,"ElementVariableViewerPanel", wxScrolledWindow);
				element_point_viewer->element_point_win=NULL;
				element_point_viewer->paneSz = NULL;
				wxBoxSizer *Collpane_sizer = new wxBoxSizer( wxVERTICAL );
				element_point_viewer->collpane->SetSizer(Collpane_sizer);
				FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
					 element_point_viewer_add_collpane,
					 (void *)element_point_viewer,
					 computed_field_manager);
				element_point_viewer->frame=
					 XRCCTRL(*element_point_viewer->wx_element_point_viewer, 
							"CmguiElementPointViewer", wxFrame);
				element_point_viewer->frame->Layout();
				element_point_viewer->frame->SetMinSize(wxSize(50,200));
				element_point_viewer->collpane->Layout();
#endif /* defined (WX_USER_INTERFACE) */
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
		FE_region_remove_callback(element_point_viewer->fe_region,
			Element_point_viewer_FE_region_change,
			(void *)element_point_viewer);
		DEACCESS(Time_object)(&(element_point_viewer->time_object));
		/* end callbacks from global element_point selection */
		Element_point_ranges_selection_remove_callback(
			element_point_viewer->element_point_ranges_selection,
			Element_point_viewer_element_point_ranges_selection_change,
			(void *)element_point_viewer);
		/* deaccess the local element_copy */
		REACCESS(FE_element)(&(element_point_viewer->element_copy),
			(struct FE_element *)NULL);
		DEALLOCATE(*element_point_viewer_address);
		if (element_point_viewer->wx_element_point_viewer)
		{
			 delete element_point_viewer->wx_element_point_viewer;
		}
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

static int Element_point_viewer_refresh_elements(
	struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED :7 June 2007

DESCRIPTION :
Updates the element shown in the chooser to match that for the current point.
==============================================================================*/
{
	int return_code;
 
	ENTER(Element_point_viewer_refresh_elements);
	if (element_point_viewer->wx_element_point_viewer)
	{
		return_code=1;
		element_point_viewer->wx_element_point_viewer->set_element_selection(element_point_viewer);
		element_point_viewer->wx_element_point_viewer->set_top_level_element_selection(element_point_viewer);
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

static int Element_point_viewer_refresh_chooser(
	struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 7 June 2007

DESCRIPTION :
Fills the widgets for choosing the element point with the current values.
==============================================================================*/
{
	int return_code;
 
	ENTER(Element_point_viewer_refresh_chooser);
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

int Element_point_viewer_bring_window_to_front(struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Pops the window for <element_poin_viewer> to the front of those visible.
??? De-iconify as well?
==============================================================================*/
{
	int return_code;

	ENTER(Element_point_viewer_bring_window_to_front);
	if (element_point_viewer->wx_element_point_viewer)
	{
		 element_point_viewer->wx_element_point_viewer->Raise();
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

int element_point_viewer_set_element_point_field(
  void *element_point_viewer_void,
	struct Element_point_ranges_identifier *element_point_identifier,
	int element_point_number,struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 6 Jube 2007

DESCRIPTION :
Sets the element_point/field being edited in the
<element_point_field_viewer_widget>.
Note that the viewer works on the element itself, not a local copy. Hence, only
pass unmanaged elements in the element_point_identifier to this widget.
==============================================================================*/
{
	 int return_code, setup_components;
	struct Element_point_viewer *element_point_viewer;

	ENTER(element_point_viewer_set_element_point_field);
	if ((element_point_viewer = static_cast<Element_point_viewer*>(element_point_viewer_void))&&
		(((struct FE_element *)NULL==element_point_identifier->element)||
			(Element_point_ranges_identifier_element_point_number_is_valid(
				element_point_identifier,element_point_number)&&field&&
				Computed_field_is_defined_in_element(field,
					element_point_identifier->element))))
	{
		 /* rebuild viewer widgets if nature of element_point or field changed */
// 		 old_editable=element_point_field_is_editable(
// 				&(element_point_viewer->element_point_identifier),
// 				element_point_viewer->current_field,number_in_xi);
// 		 new_editable=element_point_field_is_editable(
// 				element_point_identifier,field,number_in_xi);
// 		 if ((!element_point_identifier->element)||
// 				(!(element_point_viewer->element_point_identifier.element))||
// 				(field != element_point_viewer->current_field)||
// 				(field&&((element_point_viewer->number_of_components !=
// 							Computed_field_get_number_of_components(field))||
// 					 (new_editable != old_editable))))
// 		 {
				setup_components=1;
// 		 }
// 		 else
// 		 {
// 				setup_components=0;
// 		 }
		 if (element_point_identifier->element&&field)
		 {
				COPY(Element_point_ranges_identifier)(
					 &(element_point_viewer->element_point_identifier),
					 element_point_identifier);
				element_point_viewer->element_point_number=element_point_number;
				element_point_viewer->current_field=field;
				FE_element_get_numbered_xi_point(
					 element_point_identifier->element,
					 element_point_identifier->xi_discretization_mode,
					 element_point_identifier->number_in_xi,
					 element_point_identifier->exact_xi,
					 /*coordinate_field*/(struct Computed_field *)NULL,
					 /*density_field*/(struct Computed_field *)NULL,
					 element_point_number, element_point_viewer->xi,
					 /*time*/0);
		 }
		 else
		 {
				element_point_viewer->element_point_identifier.element=
					 (struct FE_element *)NULL;
				element_point_viewer->current_field=(struct Computed_field *)NULL;
		 }
		 if (setup_components)
		 {
				element_point_viewer_setup_components(
					 element_point_viewer);
		 }
		 if (element_point_identifier->element&&field)
		 {
				if (Computed_field_has_multiple_times(field))
				{
					 if (!element_point_viewer->time_object_callback)
					 {
							element_point_viewer->time_object_callback = 
								 Time_object_add_callback(element_point_viewer->time_object,
										element_point_viewer_time_change_callback,
										(void *)element_point_viewer);
					 }
				}
				else
				{
					 if (element_point_viewer->time_object_callback)
					 {
							Time_object_remove_callback(element_point_viewer->time_object,
								 element_point_viewer_time_change_callback,
								 (void *)element_point_viewer);
							element_point_viewer->time_object_callback = 0;
					 }					
				}
// 				element_point_field_viewer_widget_update_values(
// 					element_point_field_viewer);
// 				XtManageChild(element_point_field_viewer->widget);
		 }
// 		 else
// 		 {
// 				 				XtUnmanageChild(element_point_field_viewer->widget);
// 				 		 
// 				 			return_code = 1;
// 		 }
// 		 else
// 		 {
// 				display_message(ERROR_MESSAGE,
// 					 "element_point_field_viewer_widget_set_element_point_field.  "
// 					 "Missing widget data");
// 				return_code = 0;
// 		 }
	}
	else
	{
		 display_message(ERROR_MESSAGE,
				"element_point_viewer_set_element_point_field.  "
				"Invalid argument(s)");
		 return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* element_point_field_viewer_widget_set_element_point_field */

static int element_point_viewer_setup_components(
	struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 10 October 2003

DESCRIPTION :
Creates the array of cells containing field component names and values.
==============================================================================*/
{
	 char *component_name, *new_string;
	 int return_code, number_of_components, editable;
	struct Computed_field *field;
	struct FE_element *element;
	int number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS], tmp, comp_no;
	wxString tmp_string;
	
	ENTER(element_point_field_viewer_setup_components);
	if (element_point_viewer)
	{
		 return_code=1;
		 element_point_viewer->number_of_components=-1;
		 tmp = 0;
		 if ((element=element_point_viewer->element_point_identifier.element)&&
				(field=element_point_viewer->current_field)&&
				Computed_field_is_defined_in_element(field,element))
		 {
				editable = element_point_field_is_editable(
					 &(element_point_viewer->element_point_identifier),
					 field,number_in_xi);
				number_of_components=Computed_field_get_number_of_components(field);
				element_point_viewer->number_of_components=number_of_components;
				for (comp_no=0;(comp_no<number_of_components)&&return_code;comp_no++)
				{
					 /* component name label */
					 
					 if (component_name=Computed_field_get_component_name(field,comp_no))
					 {
							new_string = component_name;
							tmp_string = new_string;
							if (tmp == 0)
							{
								 tmp = 1;
								 element_point_viewer->element_point_grid_field = new wxGridSizer(2,2,1,1);
							}
							element_point_viewer->element_point_grid_field->Add(new wxStaticText(
								 element_point_viewer->element_point_win, -1, wxT(tmp_string), 
								 /*the default position is not nice in the optimised
									 version, so manually setting up the position*/
								 wxPoint(0,comp_no*35 + 5),wxDefaultSize),1,
								 wxALIGN_CENTER_VERTICAL|
								 wxALIGN_CENTER_HORIZONTAL|wxADJUST_MINSIZE, 0);
							element_point_viewer_add_textctrl(editable,element_point_viewer, field, comp_no);
							DEALLOCATE(component_name);
					 }
					 
				}
		 }
		 else
		 {
				display_message(ERROR_MESSAGE,
					 "element_point_viewer_widget_setup_components.  "
					 "Invalid argument(s)");
				return_code=0;
		 }
		 LEAVE;
	}
		
	return (return_code);
} /* element_point_field_viewer_widget_setup_components */

int element_point_viewer_widget_set_element_point(
	Element_point_viewer *element_point_viewer,
	struct Element_point_ranges_identifier *element_point_identifier,
	int element_point_number)
/*******************************************************************************
LAST MODIFIED : 6 June 2007

DESCRIPTION :
Sets the element point being edited in the <element_point_viewer_widget>. Note
that the viewer works on the element itself, not a local copy. Hence, only pass
unmanaged elements in the identifier to this widget.
==============================================================================*/
{
	 int change_conditional_function,return_code;
	 MANAGER_CONDITIONAL_FUNCTION(Computed_field)
			*choose_field_conditional_function;
	 struct CM_element_information element_identifier;
	 struct Computed_field *field;
	 struct FE_element *element,*template_element;
	 
	 ENTER(element_point_viewer_widget_set_element_point);
	 if (element_point_viewer && element_point_identifier&&
			(((struct FE_element *)NULL==element_point_identifier->element)||
				 Element_point_ranges_identifier_element_point_number_is_valid(
						element_point_identifier,element_point_number)))
	 {
			change_conditional_function=0;
			if (element = element_point_identifier->element)
			{
				 field=element_point_viewer->current_field;
				 if (!(element_point_viewer->template_element) ||
						(!equivalent_computed_fields_at_elements(element,
							 element_point_viewer->template_element)))
				 {
						choose_field_conditional_function=
							 Computed_field_is_defined_in_element_conditional;
						change_conditional_function=1;
						if ((!field)||
							 (!Computed_field_is_defined_in_element(field,element)))
						{
							 field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
									choose_field_conditional_function,(void *)element,
									Computed_field_package_get_computed_field_manager(
										 element_point_viewer->computed_field_package));
						}
						get_FE_element_identifier(element, &element_identifier);
						template_element = CREATE(FE_element)(&element_identifier,
							 (struct FE_element_shape *)NULL, (struct FE_region *)NULL, element);
				 }
			}
			else
			{
				 field=(struct Computed_field *)NULL;
				 choose_field_conditional_function=
						(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
				 change_conditional_function=1;
				 template_element=(struct FE_element *)NULL;
			}
			COPY(Element_point_ranges_identifier)(
				 &(element_point_viewer->element_point_identifier),
				 element_point_identifier);
			element_point_viewer->element_point_number=element_point_number;
			if (change_conditional_function)
			{
				 REACCESS(FE_element)(&(element_point_viewer->template_element),
						template_element);
// 				 CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION(Computed_field)(
// 						element_point_viewer->choose_field_widget,
// 						choose_field_conditional_function,
// 						(void *)element_point_viewer->template_element,field);
			}
// 			element_point_viewer_set_element_point_field(
// 				 element_point_viewer,
// 				 &(element_point_viewer->element_point_identifier),
// 				 element_point_number,field);
	 }
	 
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "element_point_viewer_widget_set_element_point.  Invalid argument(s)");
			return_code=0;
	 }
	 LEAVE;

	return (return_code);
} /* element_point_viewer_widget_set_element_point */

static int Element_point_viewer_refresh_grid_value_text(
	 struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 13 June 2007

DESCRIPTION :
Updates the grid_value text field. If there is a current element point, writes
the field value, otherwise N/A.
==============================================================================*/
{
	char *field_value_string,*value_string;
	FE_value time;
	int is_sensitive,return_code;
	struct Computed_field *grid_field;
	struct FE_element *element,*top_level_element;
 
	ENTER(Element_point_viewer_refresh_grid_value_text);
	if (element_point_viewer)
	{
		return_code=1;
		/* Get the text string */
		if (value_string = const_cast<char *>(element_point_viewer->gridvaluetextctrl->GetValue().c_str()))
		{
			 
			if ((element=element_point_viewer->element_point_identifier.element)&&
				(top_level_element=
					element_point_viewer->element_point_identifier.top_level_element)&&
				 (grid_field= element_point_viewer->wx_element_point_viewer->get_grid_field())&&
				Computed_field_is_defined_in_element(grid_field,element))
			{
				time = Time_object_get_current_time(element_point_viewer->time_object);
				if (field_value_string=Computed_field_evaluate_as_string_in_element(
					grid_field,/*component_number*/-1,element,
					element_point_viewer->xi,time,top_level_element))
				{
					/* only set string from field if different from that shown */
					if (strcmp(field_value_string,value_string))
					{
						element_point_viewer->gridvaluetextctrl->SetValue(field_value_string);
					}
					DEALLOCATE(field_value_string);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Element_point_viewer_refresh_grid_value_text.  "
						"Could not evaluate field");
					element_point_viewer->gridvaluetextctrl->SetValue("ERROR");
				}
				Computed_field_clear_cache(grid_field);
				is_sensitive=1;
			}
			else
			{
				/* only set string if different from that shown */
				if (strcmp(value_string,"N/A"))
				{
					 element_point_viewer->gridvaluetextctrl->SetValue("N/A");
				}
				is_sensitive=0;
			}
			element_point_viewer->gridvaluetextctrl->Enable(is_sensitive);
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
			 element_point_viewer->wx_element_point_viewer->
					set_xi_discretization_mode_chooser_value(element_point_viewer);
			is_sensitive=1;
		}
		else
		{
			is_sensitive=0;
		}
		element_point_viewer->xi_discretization_mode_chooser_panel->Enable(is_sensitive);
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
