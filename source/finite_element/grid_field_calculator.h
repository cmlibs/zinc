/*******************************************************************************
FILE : grid_field_calculator.h

LAST MODIFIED : 27 February 2003

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
#if !defined (GRID_FIELD_CALCULATOR_H)
#define GRID_FIELD_CALCULATOR_H

#include "curve/curve.h"
#include "computed_field/computed_field.h"
#include "finite_element/finite_element.h"
#include "region/cmiss_region.h"
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
	Widget *curve_editor_dialog_address,
	struct Cmiss_region *region,
	struct MANAGER(Curve) *curve_manager,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 27 February 2003

DESCRIPTION :
If there is a grid field calculator in existence, then bring it to the
front, else create a new one.
==============================================================================*/

#endif /* !defined (GRID_FIELD_CALCULATOR_H) */
