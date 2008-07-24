/*******************************************************************************
FILE : grid_field_calculator.c

LAST MODIFIED : 14 December 2001

DESCRIPTION :
An editor for setting values of grid fields in elements based on
control curve variation over coordinates - usually xi_texture_coordinates.
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
#include <stdio.h>
#include "choose/choose_computed_field.h"
#include "choose/choose_curve.h"
#include "choose/text_choose_fe_element.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "curve/curve.h"
#include "curve/curve_editor_dialog.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_composite.h"
#include "computed_field/computed_field_curve.h"
#include "computed_field/computed_field_integration.h"
#include "computed_field/computed_field_update.h"
#include "computed_field/computed_field_vector_operations.h"
#include "finite_element/finite_element.h"
#include "finite_element/grid_field_calculator.h"
static char grid_field_calculator_uidh[] =
#include "finite_element/grid_field_calculator.uidh"
	;
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int grid_field_calculator_hierarchy_open=0;
static MrmHierarchy grid_field_calculator_hierarchy;
#endif /* defined (MOTIF) */

struct Grid_field_calculator
/*******************************************************************************
LAST MODIFIED : 19 March 2003

DESCRIPTION :
Contains all the information needed for the grid_field_calculator dialog.
==============================================================================*/
{
	struct Computed_field_package *computed_field_package;
	Widget *curve_editor_dialog_address;
	struct MANAGER(Curve) *curve_manager;
	struct Cmiss_region *region;
	struct FE_region *fe_region;
	struct User_interface *user_interface;
	Widget coord_field_form,coord_field_widget,
		grid_field_form,grid_field_widget,
		seed_element_entry,seed_element_form,seed_element_widget,
		axis1_curve_entry,axis1_curve_form,axis1_curve_widget,
		axis2_curve_entry,axis2_curve_form,axis2_curve_widget,
		axis3_curve_entry,axis3_curve_form,axis3_curve_widget;
	Widget *dialog_address,dialog,widget,dialog_parent;
}; /* struct Grid_field_calculator */

/*
Module functions
----------------
*/

DECLARE_DIALOG_IDENTIFY_FUNCTION(grid_field_calculator, \
	Grid_field_calculator,axis1_curve_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(grid_field_calculator, \
	Grid_field_calculator,axis1_curve_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(grid_field_calculator, \
	Grid_field_calculator,axis2_curve_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(grid_field_calculator, \
	Grid_field_calculator,axis2_curve_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(grid_field_calculator, \
	Grid_field_calculator,axis3_curve_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(grid_field_calculator, \
	Grid_field_calculator,axis3_curve_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(grid_field_calculator, \
	Grid_field_calculator,coord_field_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(grid_field_calculator, \
	Grid_field_calculator,grid_field_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(grid_field_calculator, \
	Grid_field_calculator,seed_element_entry)
DECLARE_DIALOG_IDENTIFY_FUNCTION(grid_field_calculator, \
	Grid_field_calculator,seed_element_form)

static void grid_field_calculator_destroy_CB(Widget widget,
	XtPointer client_data,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 30 November 1999

DESCRIPTION :
Callback for when the grid_field_calculator is closed. Tidies up all
details: dynamic allocations, etc.
==============================================================================*/
{
	struct Grid_field_calculator *grid_calc;

	ENTER(grid_field_calculator_destroy_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(reason);
	if (grid_calc=(struct Grid_field_calculator *)client_data)
	{
		/* clear the grid_field_calculator for the client */
		*(grid_calc->dialog_address)=(Widget)NULL;
		/* deallocate the memory for the user data */
		DEALLOCATE(grid_calc);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"grid_field_calculator_destroy_CB.  Missing dialog");
	}
	LEAVE;
} /* grid_field_calculator_destroy_CB */

static void grid_field_calculator_axis1_curve_edit_CB(Widget widget,
	XtPointer client_data,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 3 December 1999

DESCRIPTION :
==============================================================================*/
{
	char *curve_name;
	int error;
	struct Computed_field *grid_field;
	struct Curve *current_curve,*curve;
	struct Grid_field_calculator *grid_calc;

	ENTER(grid_field_calculator_axis1_curve_edit_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(reason);
	if (grid_calc=(struct Grid_field_calculator *)client_data)
	{
		/* edit the curve in the chooser... */
		if (curve=CHOOSE_OBJECT_GET_OBJECT(Curve)(
			grid_calc->axis1_curve_widget))
		{
			current_curve=curve;
			/* ...but not if it is called "constant_1" */
			if (curve==FIND_BY_IDENTIFIER_IN_MANAGER(Curve,name)(
				"constant_1",grid_calc->curve_manager))
			{
				if (grid_field=CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
					grid_calc->grid_field_widget))
				{
					error=0;
					if (GET_NAME(Computed_field)(grid_field,&curve_name)&&
						append_string(&curve_name,"_axis1_curve",&error))
					{
						/* try to find curve called curve_name */
						if (!(curve=FIND_BY_IDENTIFIER_IN_MANAGER(Curve,name)(
							curve_name,grid_calc->curve_manager)))
						{
							if (curve=CREATE(Curve)(curve_name,LINEAR_LAGRANGE,1))
							{
								if (!(MANAGER_COPY_WITHOUT_IDENTIFIER(Curve,name)(curve,
									current_curve)&&
									ADD_OBJECT_TO_MANAGER(Curve)(curve,
										grid_calc->curve_manager)))
								{
									DESTROY(Curve)(&curve);
								}
							}
						}
						CHOOSE_OBJECT_SET_OBJECT(Curve)(
							grid_calc->axis1_curve_widget,curve);
						DEALLOCATE(curve_name);
					}
				}
			}
		}
		bring_up_curve_editor_dialog(
			grid_calc->curve_editor_dialog_address,
			User_interface_get_application_shell(grid_calc->user_interface),
			grid_calc->curve_manager,curve,grid_calc->user_interface);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"grid_field_calculator_axis1_curve_edit_CB.  Missing dialog");
	}
	LEAVE;
} /* grid_field_calculator_axis1_curve_edit_CB */

static void grid_field_calculator_axis2_curve_edit_CB(Widget widget,
	XtPointer client_data,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 3 December 1999

DESCRIPTION :
==============================================================================*/
{
	char *curve_name;
	int error;
	struct Computed_field *grid_field;
	struct Curve *current_curve,*curve;
	struct Grid_field_calculator *grid_calc;

	ENTER(grid_field_calculator_axis2_curve_edit_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(reason);
	if (grid_calc=(struct Grid_field_calculator *)client_data)
	{
		/* edit the curve in the chooser... */
		if (curve=current_curve=CHOOSE_OBJECT_GET_OBJECT(Curve)(
			grid_calc->axis2_curve_widget))
		{
			/* ...but not if it is called "constant_1" */
			if (curve==FIND_BY_IDENTIFIER_IN_MANAGER(Curve,name)(
				"constant_1",grid_calc->curve_manager))
			{
				if (grid_field=CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
					grid_calc->grid_field_widget))
				{
					error=0;
					if (GET_NAME(Computed_field)(grid_field,&curve_name)&&
						append_string(&curve_name,"_axis2_curve",&error))
					{
						/* try to find curve called curve_name */
						if (!(curve=FIND_BY_IDENTIFIER_IN_MANAGER(Curve,name)(
							curve_name,grid_calc->curve_manager)))
						{
							if (curve=CREATE(Curve)(curve_name,LINEAR_LAGRANGE,1))
							{
								if (!(MANAGER_COPY_WITHOUT_IDENTIFIER(Curve,name)(curve,
									current_curve)&&
									ADD_OBJECT_TO_MANAGER(Curve)(curve,
										grid_calc->curve_manager)))
								{
									DESTROY(Curve)(&curve);
								}
							}
						}
						CHOOSE_OBJECT_SET_OBJECT(Curve)(
							grid_calc->axis2_curve_widget,curve);
						DEALLOCATE(curve_name);
					}
				}
			}
		}
		bring_up_curve_editor_dialog(
			grid_calc->curve_editor_dialog_address,
			User_interface_get_application_shell(grid_calc->user_interface),
			grid_calc->curve_manager,curve,grid_calc->user_interface);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"grid_field_calculator_axis2_curve_edit_CB.  Missing dialog");
	}
	LEAVE;
} /* grid_field_calculator_axis2_curve_edit_CB */

static void grid_field_calculator_axis3_curve_edit_CB(Widget widget,
	XtPointer client_data,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 3 December 1999

DESCRIPTION :
==============================================================================*/
{
	char *curve_name;
	int error;
	struct Computed_field *grid_field;
	struct Curve *current_curve,*curve;
	struct Grid_field_calculator *grid_calc;

	ENTER(grid_field_calculator_axis3_curve_edit_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(reason);
	if (grid_calc=(struct Grid_field_calculator *)client_data)
	{
		/* edit the curve in the chooser... */
		if (curve=current_curve=CHOOSE_OBJECT_GET_OBJECT(Curve)(
			grid_calc->axis3_curve_widget))
		{
			/* ...but not if it is called "constant_1" */
			if (curve==FIND_BY_IDENTIFIER_IN_MANAGER(Curve,name)(
				"constant_1",grid_calc->curve_manager))
			{
				if (grid_field=CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
					grid_calc->grid_field_widget))
				{
					error=0;
					if (GET_NAME(Computed_field)(grid_field,&curve_name)&&
						append_string(&curve_name,"_axis3_curve",&error))
					{
						/* try to find curve called curve_name */
						if (!(curve=FIND_BY_IDENTIFIER_IN_MANAGER(Curve,name)(
							curve_name,grid_calc->curve_manager)))
						{
							if (curve=CREATE(Curve)(curve_name,LINEAR_LAGRANGE,1))
							{
								if (!(MANAGER_COPY_WITHOUT_IDENTIFIER(Curve,name)(curve,
									current_curve)&&
									ADD_OBJECT_TO_MANAGER(Curve)(curve,
										grid_calc->curve_manager)))
								{
									DESTROY(Curve)(&curve);
								}
							}
						}
						CHOOSE_OBJECT_SET_OBJECT(Curve)(
							grid_calc->axis3_curve_widget,curve);
						DEALLOCATE(curve_name);
					}
				}
			}
		}
		bring_up_curve_editor_dialog(
			grid_calc->curve_editor_dialog_address,
			User_interface_get_application_shell(grid_calc->user_interface),
			grid_calc->curve_manager,curve,grid_calc->user_interface);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"grid_field_calculator_axis3_curve_edit_CB.  Missing dialog");
	}
	LEAVE;
} /* grid_field_calculator_axis3_curve_edit_CB */

int grid_field_calculator_apply(struct Grid_field_calculator *grid_calc)
/*******************************************************************************
LAST MODIFIED : 14 December 2001

DESCRIPTION :
Applies the computed field to the grid field.
==============================================================================*/
{
	char tmp_string[20];
	int axis,i,number_of_components,return_code;
	struct Computed_field *coordinate_component[3],*coordinate_field,
		*curve_lookup[3],*grid_field,*source_field,*temp_field;
	struct MANAGER(Computed_field) *computed_field_manager;
	Widget widget;

	ENTER(grid_field_calculator_apply);
	if (grid_calc&&(computed_field_manager=
		Computed_field_package_get_computed_field_manager(
			grid_calc->computed_field_package)))
	{
		return_code=1;
		if (grid_field=CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
			grid_calc->grid_field_widget))
		{
			if (coordinate_field=CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
				grid_calc->coord_field_widget))
			{
				number_of_components=
					Computed_field_get_number_of_components(coordinate_field);
				for (i=0;i<number_of_components;i++)
				{
					axis=i+1;
					switch (axis)
					{
						case 1:
						{
							widget=grid_calc->axis1_curve_widget;
						} break;
						case 2:
						{
							widget=grid_calc->axis2_curve_widget;
						} break;
						case 3:
						{
							widget=grid_calc->axis3_curve_widget;
						} break;
					}
					/* get or make wrapper for field component */
					coordinate_component[i] =
						Computed_field_manager_get_component_wrapper(
							computed_field_manager, coordinate_field, i);
					sprintf(tmp_string,"curve_lookup%d",axis);
					if (curve_lookup[i]=CREATE(Computed_field)(tmp_string))
					{
						ACCESS(Computed_field)(curve_lookup[i]);
						if (!Computed_field_set_type_curve_lookup(curve_lookup[i],
							coordinate_component[i],
							CHOOSE_OBJECT_GET_OBJECT(Curve)(widget),
							grid_calc->curve_manager))
						{
							return_code = 0;
						}
					}
					else
					{
						return_code=0;
					}
				}
				if (return_code)
				{
					if (1==number_of_components)
					{
						source_field=curve_lookup[0];
					}
					else
					{
						if (source_field=CREATE(Computed_field)("dot_product1"))
						{
							if (Computed_field_set_type_dot_product(source_field,
								curve_lookup[0],curve_lookup[1]))
							{
								if (3==number_of_components)
								{
									temp_field=source_field;
									if (source_field=CREATE(Computed_field)("dot_product2"))
									{
										if (!Computed_field_set_type_dot_product(source_field,
											temp_field,curve_lookup[2]))
										{
											DESTROY(Computed_field)(&source_field);
										}
									}
									else
									{
										DESTROY(Computed_field)(&temp_field);
									}
								}
							}
							else
							{
								DESTROY(Computed_field)(&source_field);
							}
						}
					}
					if (source_field)
					{
						ACCESS(Computed_field)(source_field);
						return_code=Computed_field_update_element_values_from_source(
							grid_field,source_field, grid_calc->region,
							(struct Element_point_ranges_selection *)NULL,
							(struct FE_element_selection *)NULL, /*time*/0);
						DEACCESS(Computed_field)(&source_field);
					}
					else
					{
						return_code=0;
					}
				}
				for (i=0;i<number_of_components;i++)
				{
					if (curve_lookup[i])
					{
						DEACCESS(Computed_field)(&(curve_lookup[i]));
					}
				}
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"Could not set grid values - field may not be grid based");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"grid_field_calculator_apply.  No grid field");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"grid_field_calculator_apply.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* grid_field_calculator_apply */

static void grid_field_calculator_ok_button_CB(Widget widget,
	XtPointer client_data,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 3 December 1999

DESCRIPTION :
==============================================================================*/
{
	struct Grid_field_calculator *grid_calc;

	ENTER(grid_field_calculator_ok_button_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(reason);
	if (grid_calc=(struct Grid_field_calculator *)client_data)
	{
		if (grid_field_calculator_apply(grid_calc))
		{
			/* close the dialog shell */
			XtDestroyWidget(grid_calc->dialog);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"grid_field_calculator_ok_button_CB.  Missing dialog");
	}
	LEAVE;
} /* grid_field_calculator_ok_button_CB */

static void grid_field_calculator_apply_button_CB(Widget widget,
	XtPointer client_data,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 3 December 1999

DESCRIPTION :
==============================================================================*/
{
	struct Grid_field_calculator *grid_calc;

	ENTER(grid_field_calculator_apply_button_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(reason);
	if (grid_calc=(struct Grid_field_calculator *)client_data)
	{
		grid_field_calculator_apply(grid_calc);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"grid_field_calculator_apply_button_CB.  Missing dialog");
	}
	LEAVE;
} /* grid_field_calculator_apply_button_CB */

static void grid_field_calculator_cancel_button_CB(Widget widget,
	XtPointer client_data,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 30 November 1999

DESCRIPTION :
==============================================================================*/
{
	struct Grid_field_calculator *grid_calc;

	ENTER(grid_field_calculator_cancel_button_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(reason);
	if (grid_calc=(struct Grid_field_calculator *)client_data)
	{
		/* close the dialog shell */
		XtDestroyWidget(grid_calc->dialog);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"grid_field_calculator_cancel_button_CB.  Missing dialog");
	}
	LEAVE;
} /* grid_field_calculator_cancel_button_CB */

static int grid_field_calculator_set_grid_field(
	struct Grid_field_calculator *grid_calc)
/*******************************************************************************
LAST MODIFIED : 18 September 2001

DESCRIPTION :
Sets the dialog to look at <grid_field>. Establishes coordinate_field
"xi_texture_coordinates" and default curve "constant_1_curve".
==============================================================================*/
{
	char *curve_name,*field_name;
	double double_value;
	FE_value value;
	int axis, integration_magnitude_coordinates, return_code;
	struct Computed_field *coordinate_field, *grid_field, *integration_integrand,
		*integration_coordinate_field;
	struct Curve *constant_1_curve,*curve;
	struct FE_element *seed_element;
	struct MANAGER(Computed_field) *computed_field_manager;
	Widget widget;

	ENTER(grid_field_calculator_set_grid_field);
	if (grid_calc&&(computed_field_manager=
		Computed_field_package_get_computed_field_manager(
			grid_calc->computed_field_package)))
	{
		return_code=1;
		integration_coordinate_field = (struct Computed_field *)NULL;
		integration_integrand = (struct Computed_field *)NULL;
		if (coordinate_field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
			"xi_texture_coordinates",computed_field_manager))
		{
			if (Computed_field_is_type_integration(coordinate_field)&&
				Computed_field_get_type_integration(coordinate_field,
					&seed_element,&integration_integrand,
					&integration_magnitude_coordinates, &integration_coordinate_field))
			{
				TEXT_CHOOSE_FROM_FE_REGION_SET_OBJECT(FE_element)(
					grid_calc->seed_element_widget,seed_element);
			}
		}
		else
		{
			if (!integration_coordinate_field)
			{
				integration_coordinate_field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)
					("xi", computed_field_manager);
			}
			double_value = 1.0;
			if (!integration_integrand)
			{
				integration_integrand = Computed_field_create_constant(1,&double_value);
				Computed_field_set_name(integration_integrand, "constant_1.0");
			}
			else
			{
				ACCESS(Computed_field)(integration_integrand);
			}
			if (seed_element=TEXT_CHOOSE_FROM_FE_REGION_GET_OBJECT(FE_element)(
				grid_calc->seed_element_widget))
			{
				if (coordinate_field=CREATE(Computed_field)("xi_texture_coordinates"))
				{
					integration_magnitude_coordinates = 0;
					if (!(Computed_field_set_type_integration(
						coordinate_field,seed_element,grid_calc->fe_region,
						integration_integrand,
						integration_magnitude_coordinates,integration_coordinate_field)&&
						ADD_OBJECT_TO_MANAGER(Computed_field)(coordinate_field,
							computed_field_manager)))
					{
						DESTROY(Computed_field)(&coordinate_field);
					}
				}
			}
			DEACCESS(Computed_field)(&integration_integrand);
		}
		if (coordinate_field)
		{
			CHOOSE_OBJECT_SET_OBJECT(Computed_field)(grid_calc->coord_field_widget,
				coordinate_field);
			XtSetSensitive(grid_calc->seed_element_entry,
				Computed_field_is_type_integration(coordinate_field));
		}
		if (!(constant_1_curve=FIND_BY_IDENTIFIER_IN_MANAGER(Curve,name)(
			"constant_1",grid_calc->curve_manager)))
		{
			if (constant_1_curve=CREATE(Curve)("constant_1",
				LINEAR_LAGRANGE,1))
			{
				Curve_add_element(constant_1_curve,1);
				Curve_set_parameter(constant_1_curve,1,0,0.0);
				Curve_set_parameter(constant_1_curve,1,1,1.0);
				value=1.0;
				Curve_set_node_values(constant_1_curve,1,0,&value);
				Curve_set_node_values(constant_1_curve,1,1,&value);
				if (!ADD_OBJECT_TO_MANAGER(Curve)(constant_1_curve,
					grid_calc->curve_manager))
				{
					DESTROY(Curve)(&constant_1_curve);
				}
			}
		}
		if ((grid_field=CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
			grid_calc->grid_field_widget))&&
			GET_NAME(Computed_field)(grid_field,&field_name))
		{
			if (ALLOCATE(curve_name,char,strlen(field_name)+13))
			{
				for (axis=1;axis<=3;axis++)
				{
					switch (axis)
					{
						case 1:
						{
							widget=grid_calc->axis1_curve_widget;
						} break;
						case 2:
						{
							widget=grid_calc->axis2_curve_widget;
						} break;
						case 3:
						{
							widget=grid_calc->axis3_curve_widget;
						} break;
					}
					sprintf(curve_name,"%s_axis%d_curve",field_name,axis);
					if (!(curve=FIND_BY_IDENTIFIER_IN_MANAGER(Curve,name)(
						curve_name,grid_calc->curve_manager)))
					{
						curve=constant_1_curve;
					}
					CHOOSE_OBJECT_SET_OBJECT(Curve)(widget,curve);
				}
				DEALLOCATE(curve_name);
			}
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"grid_field_calculator_set_grid_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* grid_field_calculator_set_grid_field */

static void grid_field_calculator_update_grid_field(Widget widget,
	void *grid_calc_void,void *grid_field_void)
/*******************************************************************************
LAST MODIFIED : 3 December 1999

DESCRIPTION :
Callback for change of grid field.
==============================================================================*/
{
	struct Grid_field_calculator *grid_calc;

	ENTER(grid_field_calculator_update_grid_field);
	USE_PARAMETER(widget);
	USE_PARAMETER(grid_field_void);
	if (grid_calc=(struct Grid_field_calculator *)grid_calc_void)
	{
		grid_field_calculator_set_grid_field(grid_calc);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"grid_field_calculator_update_grid_field.  Invalid argument(s)");
	}
	LEAVE;
} /* grid_field_calculator_update_grid_field */

static void grid_field_calculator_update_coordinate_field(Widget widget,
	void *grid_calc_void,void *coordinate_field_void)
/*******************************************************************************
LAST MODIFIED : 3 December 1999

DESCRIPTION :
Callback for change of coordinate field.
==============================================================================*/
{
	int integration_magnitude_coordinates;
	struct Computed_field *coordinate_field, *integration_integrand, 
		*integration_coordinate_field;
	struct FE_element *seed_element;
	struct Grid_field_calculator *grid_calc;

	ENTER(grid_field_calculator_update_coordinate_field);
	USE_PARAMETER(widget);
	if ((grid_calc=(struct Grid_field_calculator *)grid_calc_void)&&
		(coordinate_field=(struct Computed_field *)coordinate_field_void))
	{
		if (Computed_field_is_type_integration(coordinate_field)&&
			Computed_field_get_type_integration(coordinate_field,
				&seed_element, &integration_integrand,
				&integration_magnitude_coordinates, &integration_coordinate_field))
		{
			TEXT_CHOOSE_FROM_FE_REGION_SET_OBJECT(FE_element)(
				grid_calc->seed_element_widget,seed_element);
		}
		XtSetSensitive(grid_calc->seed_element_entry,
			Computed_field_is_type_integration(coordinate_field));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"grid_field_calculator_update_coordinate_field.  Invalid argument(s)");
	}
	LEAVE;
} /* grid_field_calculator_update_coordinate_field */

static void grid_field_calculator_update_seed_element(Widget widget,
	void *grid_calc_void,void *seed_element_void)
/*******************************************************************************
LAST MODIFIED : 3 December 1999

DESCRIPTION :
Callback for change of seed_element.
==============================================================================*/
{
	int integration_magnitude_coordinates;
	double value;
	struct Computed_field *coordinate_field,*integration_integrand,
		*integration_coordinate_field,*temp_field;
	struct FE_element *seed_element;
	struct Grid_field_calculator *grid_calc;

	ENTER(grid_field_calculator_update_coordinate_field);
	USE_PARAMETER(widget);
	if ((grid_calc=(struct Grid_field_calculator *)grid_calc_void)&&
		(seed_element=(struct FE_element *)seed_element_void))
	{
		if (coordinate_field=
			CHOOSE_OBJECT_GET_OBJECT(Computed_field)(grid_calc->coord_field_widget))
		{
			if (Computed_field_is_type_integration(coordinate_field))
			{
				Computed_field_get_type_integration(coordinate_field,
					&seed_element,&integration_integrand,
					&integration_magnitude_coordinates,&integration_coordinate_field);
				ACCESS(Computed_field)(integration_integrand);
			}
			else
			{
				integration_coordinate_field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)
					("xi", Computed_field_package_get_computed_field_manager(
							grid_calc->computed_field_package));
				value = 1.0;
				integration_integrand = Computed_field_create_constant(1,&value);
				Computed_field_set_name(integration_integrand,"constant_1.0");
				integration_magnitude_coordinates = 0;
			}
			/* this is very inefficient - creates xi mapping twice per element! */
			if (temp_field=CREATE(Computed_field)("temp"))
			{
				if (Computed_field_set_type_integration(temp_field,
					seed_element,grid_calc->fe_region,integration_integrand,
					integration_magnitude_coordinates,integration_coordinate_field))
				{
					MANAGER_MODIFY_NOT_IDENTIFIER(Computed_field,name)(
						coordinate_field,temp_field,
						Computed_field_package_get_computed_field_manager(
							grid_calc->computed_field_package));
				}
				DESTROY(Computed_field)(&temp_field);
			}
			DEACCESS(Computed_field)(&integration_integrand);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"grid_field_calculator_update_.  Invalid argument(s)");
	}
	LEAVE;
} /* grid_field_calculator_update_ */

static Widget create_grid_field_calculator(
	Widget *grid_field_calculator_address,Widget parent,
	struct Computed_field_package *computed_field_package,
	Widget *curve_editor_dialog_address,
	struct MANAGER(Curve) *curve_manager,
	struct Cmiss_region *region,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 19 March 2003

DESCRIPTION :
Creates an editor for setting values of grid fields in elements based on
control curve variation over coordinates - usually integration.
==============================================================================*/
{
	int init_widgets;
	MrmType grid_field_calculator_dialog_class;
	static MrmRegisterArg callback_list[]=
	{
		{"grid_calc_id_grid_field_form",(XtPointer)
			DIALOG_IDENTIFY(grid_field_calculator,grid_field_form)},
		{"grid_calc_id_coord_field_form",(XtPointer)
			DIALOG_IDENTIFY(grid_field_calculator,coord_field_form)},
		{"grid_calc_id_seed_element_entry",(XtPointer)
			DIALOG_IDENTIFY(grid_field_calculator,seed_element_entry)},
		{"grid_calc_id_seed_element_form",(XtPointer)
			DIALOG_IDENTIFY(grid_field_calculator,seed_element_form)},
		{"grid_calc_id_axis1_curve_entry",(XtPointer)
			DIALOG_IDENTIFY(grid_field_calculator,axis1_curve_entry)},
		{"grid_calc_id_axis1_curve_form",(XtPointer)
			DIALOG_IDENTIFY(grid_field_calculator,axis1_curve_form)},
		{"grid_calc_id_axis2_curve_entry",(XtPointer)
			DIALOG_IDENTIFY(grid_field_calculator,axis2_curve_entry)},
		{"grid_calc_id_axis2_curve_form",(XtPointer)
			DIALOG_IDENTIFY(grid_field_calculator,axis2_curve_form)},
		{"grid_calc_id_axis3_curve_entry",(XtPointer)
			DIALOG_IDENTIFY(grid_field_calculator,axis3_curve_entry)},
		{"grid_calc_id_axis3_curve_form",(XtPointer)
			DIALOG_IDENTIFY(grid_field_calculator,axis3_curve_form)},
		{"grid_calc_destroy_CB",(XtPointer)
			grid_field_calculator_destroy_CB},
		{"grid_calc_axis1_curve_edit_CB",(XtPointer)
			grid_field_calculator_axis1_curve_edit_CB},
		{"grid_calc_axis2_curve_edit_CB",(XtPointer)
			grid_field_calculator_axis2_curve_edit_CB},
		{"grid_calc_axis3_curve_edit_CB",(XtPointer)
			grid_field_calculator_axis3_curve_edit_CB},
		{"grid_calc_ok_btn_CB",(XtPointer)
			grid_field_calculator_ok_button_CB},
		{"grid_calc_apply_btn_CB",(XtPointer)
			grid_field_calculator_apply_button_CB},
		{"grid_calc_cancel_btn_CB",(XtPointer)
			grid_field_calculator_cancel_button_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"grid_calc_structure",(XtPointer)NULL}
	};
	struct Callback_data callback;
	struct Grid_field_calculator *grid_calc;
	struct MANAGER(Computed_field) *computed_field_manager;
	Widget return_widget;

	ENTER(create_grid_field_calculator);
	return_widget=(Widget)NULL;
	if (grid_field_calculator_address&&parent&&computed_field_package&&
		(computed_field_manager=Computed_field_package_get_computed_field_manager(
			computed_field_package))&&
		curve_manager&&region&&user_interface&&
		curve_editor_dialog_address)
	{
		if (MrmOpenHierarchy_binary_string(grid_field_calculator_uidh, sizeof(grid_field_calculator_uidh),
			&grid_field_calculator_hierarchy,
			&grid_field_calculator_hierarchy_open))
		{
			if (ALLOCATE(grid_calc,struct Grid_field_calculator,1))
			{
				/* initialise the structure */
				grid_calc->dialog_parent = parent;
				grid_calc->dialog_address = grid_field_calculator_address;
				grid_calc->computed_field_package = computed_field_package;
				grid_calc->curve_manager = curve_manager;
				grid_calc->curve_editor_dialog_address =
					curve_editor_dialog_address;
				grid_calc->region = region;
				grid_calc->fe_region = Cmiss_region_get_FE_region(region);
				grid_calc->user_interface = user_interface;
				grid_calc->axis1_curve_entry = (Widget)NULL;
				grid_calc->axis1_curve_form = (Widget)NULL;
				grid_calc->axis1_curve_widget =(Widget)NULL;
				grid_calc->axis2_curve_entry = (Widget)NULL;
				grid_calc->axis2_curve_form = (Widget)NULL;
				grid_calc->axis2_curve_widget = (Widget)NULL;
				grid_calc->axis3_curve_entry = (Widget)NULL;
				grid_calc->axis3_curve_form = (Widget)NULL;
				grid_calc->axis3_curve_widget = (Widget)NULL;
				grid_calc->coord_field_form = (Widget)NULL;
				grid_calc->coord_field_widget = (Widget)NULL;
				grid_calc->grid_field_form = (Widget)NULL;
				grid_calc->grid_field_widget = (Widget)NULL;
				grid_calc->seed_element_entry = (Widget)NULL;
				grid_calc->seed_element_form = (Widget)NULL;
				grid_calc->seed_element_widget = (Widget)NULL;
				grid_calc->widget = (Widget)NULL;
				grid_calc->dialog = (Widget)NULL;
				/* make the dialog shell */
				if (grid_calc->dialog=XtVaCreatePopupShell(
					"Grid Field Calculator",topLevelShellWidgetClass,parent,
					XmNallowShellResize,FALSE,NULL))
				{
					/* register the callbacks */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						grid_field_calculator_hierarchy,callback_list,
						XtNumber(callback_list)))
					{
						/* assign and register the identifiers */
						identifier_list[0].value=(XtPointer)grid_calc;
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
							grid_field_calculator_hierarchy,identifier_list,
							XtNumber(identifier_list)))
						{
							/* fetch dialog widget */
							if (MrmSUCCESS==MrmFetchWidget(grid_field_calculator_hierarchy,
								"grid_calc_widget",grid_calc->dialog,&(grid_calc->widget),
								&grid_field_calculator_dialog_class))
							{
								XtManageChild(grid_calc->widget);
								init_widgets=1;
								/* create subwidgets */
								if (!(grid_calc->grid_field_widget=
									CREATE_CHOOSE_OBJECT_WIDGET(Computed_field)(
										grid_calc->grid_field_form,(struct Computed_field *)NULL,
										computed_field_manager,Computed_field_is_scalar,
										(void *)NULL, user_interface)))
								{
									init_widgets=0;
								}
								if (!(grid_calc->coord_field_widget=
									CREATE_CHOOSE_OBJECT_WIDGET(Computed_field)(
										grid_calc->coord_field_form,(struct Computed_field *)NULL,
										computed_field_manager,
										Computed_field_has_up_to_3_numerical_components,
										(void *)NULL, user_interface)))
								{
									init_widgets=0;
								}
								if (!(grid_calc->seed_element_widget=
									CREATE_TEXT_CHOOSE_FROM_FE_REGION_WIDGET(FE_element)(
										grid_calc->seed_element_form,
										(struct FE_element *)NULL,grid_calc->fe_region,
										FE_element_is_top_level,(void *)NULL,
										FE_element_to_element_string,
										FE_region_any_element_string_to_FE_element)))
								{
									init_widgets=0;
								}
								if (!(grid_calc->axis1_curve_widget=
									CREATE_CHOOSE_OBJECT_WIDGET(Curve)(
										grid_calc->axis1_curve_form,(struct Curve *)NULL,
										curve_manager,Curve_has_1_component,
										(void *)NULL, user_interface)))
								{
									init_widgets=0;
								}
								if (!(grid_calc->axis2_curve_widget=
									CREATE_CHOOSE_OBJECT_WIDGET(Curve)(
										grid_calc->axis2_curve_form,(struct Curve *)NULL,
										curve_manager,Curve_has_1_component,
										(void *)NULL, user_interface)))
								{
									init_widgets=0;
								}
								if (!(grid_calc->axis3_curve_widget=
									CREATE_CHOOSE_OBJECT_WIDGET(Curve)(
										grid_calc->axis3_curve_form,(struct Curve *)NULL,
										curve_manager,Curve_has_1_component,
										(void *)NULL, user_interface)))
								{
									init_widgets=0;
								}
								if (init_widgets)
								{
									grid_field_calculator_set_grid_field(grid_calc);
									/* turn on callbacks */
									callback.data=(void *)grid_calc;
									callback.procedure=grid_field_calculator_update_grid_field;
									CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
										grid_calc->grid_field_widget,&callback);
									callback.procedure=
										grid_field_calculator_update_coordinate_field;
									CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
										grid_calc->coord_field_widget,&callback);
									callback.procedure=
										grid_field_calculator_update_seed_element;
									TEXT_CHOOSE_FROM_FE_REGION_SET_CALLBACK(FE_element)(
										grid_calc->seed_element_widget,&callback);
									/* register for control curve manager callbacks */
									XtRealizeWidget(grid_calc->dialog);
									XtPopup(grid_calc->dialog,XtGrabNone);
									return_widget=grid_calc->dialog;
								}
								else
								{
									XtDestroyWidget(grid_calc->dialog);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"create_grid_field_calculator.  "
									"Could not fetch grid_field_calculator");
								DEALLOCATE(grid_calc);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_grid_field_calculator.  "
								"Could not register identifiers");
							DEALLOCATE(grid_calc);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_grid_field_calculator.  Could not register callbacks");
						DEALLOCATE(grid_calc);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_grid_field_calculator.  Could not create popup shell");
					DEALLOCATE(grid_calc);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_grid_field_calculator.  "
					"Could not allocate grid_field_calculator");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_grid_field_calculator.  Could not open hierarchy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_grid_field_calculator.  Invalid argument(s)");
	}
	if (grid_field_calculator_address&&return_widget)
	{
		*grid_field_calculator_address=return_widget;
	}
	LEAVE;

	return (return_widget);
} /* create_grid_field_calculator */

/*
Global functions
----------------
*/
int bring_up_grid_field_calculator(
	Widget *grid_field_calculator_address,Widget parent,
	struct Computed_field_package *computed_field_package,
	Widget *curve_editor_dialog_address,
	struct Cmiss_region *region, 
	struct MANAGER(Curve) *curve_manager,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 19 March 2003

DESCRIPTION :
If there is a grid field calculator in existence, then bring it to the
front, else create a new one.
==============================================================================*/
{
	int return_code;

	ENTER(bring_up_grid_field_calculator);
	if (grid_field_calculator_address)
	{
		if (*grid_field_calculator_address)
		{
			XtPopup(*grid_field_calculator_address,XtGrabNone);
			return_code=1;
		}
		else
		{
			if (create_grid_field_calculator(
				grid_field_calculator_address,parent,computed_field_package,
				curve_editor_dialog_address,curve_manager,
				region,user_interface))
			{
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"bring_up_grid_field_calculator.  Error creating dialog");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"bring_up_grid_field_calculator.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* bring_up_grid_field_calculator */
