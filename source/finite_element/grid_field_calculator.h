/*******************************************************************************
FILE : grid_field_calculator.h

LAST MODIFIED : 1 December 1999

DESCRIPTION :
An editor for setting values of grid fields in elements based on
control curve variation over coordinates - usually xi_texture_coordinates.
==============================================================================*/
#if !defined (GRID_FIELD_CALCULATOR_H)
#define GRID_FIELD_CALCULATOR_H

#include "curve/control_curve.h"
#include "computed_field/computed_field.h"
#include "finite_element/finite_element.h"
#include "user_interface/user_interface.h"

/*
Global Types
------------
*/

/*
Global Functions
----------------
*/
int bring_up_grid_field_calculator(
	Widget *grid_field_calculator_address,Widget parent,
	struct Computed_field_package *computed_field_package,
	Widget *control_curve_editor_dialog_address,
	struct MANAGER(Control_curve) *control_curve_manager,
	struct MANAGER(FE_element) *element_manager,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 1 December 1999

DESCRIPTION :
If there is a grid field calculator in existence, then bring it to the
front, else create a new one.
==============================================================================*/
#endif /* !defined (GRID_FIELD_CALCULATOR_H) */
