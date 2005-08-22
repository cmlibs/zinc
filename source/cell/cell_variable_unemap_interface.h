/*******************************************************************************
FILE : cell_variable_unemap_interface.h

LAST MODIFIED : 06 November 2000

DESCRIPTION :
The interface between Cell_variable's and UnEMAP
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
#if !defined (CELL_VARIABLE_UNEMAP_INTERFACE_H)
#define CELL_VARIABLE_UNEMAP_INTERFACE_H

#include "cell/cell_cmiss_interface.h"

/*
Module objects
--------------
*/
struct Cell_variable_unemap_interface;
/*******************************************************************************
LAST MODIFIED : 01 November 2000

DESCRIPTION :
A data object which stores all the information which enables Cell_variable's
to access UnEMAP
==============================================================================*/

/*
Global objects
--------------
*/

/*
Global functions
----------------
*/
struct Cell_variable_unemap_interface *CREATE(Cell_variable_unemap_interface)(
  enum Cell_value_type value_type,char *name);
/*******************************************************************************
LAST MODIFIED : 01 November 2000

DESCRIPTION :
Creates a Cell_variable_unemap_interface object.
==============================================================================*/
int DESTROY(Cell_variable_unemap_interface)(
  struct Cell_variable_unemap_interface
  **cell_variable_unemap_interface_address);
/*******************************************************************************
LAST MODIFIED : 01 November 2000

DESCRIPTION :
Destroys a Cell_variable_unemap_interface object.
==============================================================================*/
int Cell_variable_unemap_interface_add_value(
  struct Cell_variable_unemap_interface *unemap_interface,
  union Cell_cmiss_interface_value_address *value_address);
/*******************************************************************************
LAST MODIFIED : 03 November 2000

DESCRIPTION :
Adds the given <value> to the end of the <unemap_interface> time history
array. Allocates the memory required.
==============================================================================*/
int Cell_variable_unemap_interface_reset_values(
  struct Cell_variable_unemap_interface *unemap_interface);
/*******************************************************************************
LAST MODIFIED : 06 November 2000

DESCRIPTION :
Resets the value array of the given variable <unemap_interface>.
==============================================================================*/
char *Cell_variable_unemap_interface_get_value_as_string_at_position(
  struct Cell_variable_unemap_interface *unemap_interface,int position);
/*******************************************************************************
LAST MODIFIED : 03 November 2000

DESCRIPTION :
Returns the value of the given <unemap_interface> at the specified position as
a string.
==============================================================================*/
float Cell_variable_unemap_interface_get_value_as_float_at_position(
  struct Cell_variable_unemap_interface *unemap_interface,int position);
/*******************************************************************************
LAST MODIFIED : 03 November 2000

DESCRIPTION :
Returns the value of the given <unemap_interface> at the specified position as
a float.
==============================================================================*/
int Cell_variable_unemap_interface_get_number_of_values(
  struct Cell_variable_unemap_interface *unemap_interface);
/*******************************************************************************
LAST MODIFIED : 04 November 2000

DESCRIPTION :
Returns the number of values stored in the <unemap_interface>
==============================================================================*/
char *Cell_variable_unemap_interface_get_name(
  struct Cell_variable_unemap_interface *unemap_interface);
/*******************************************************************************
LAST MODIFIED : 05 November 2000

DESCRIPTION :
Returns a copy of the name of the <unemap_interface>
==============================================================================*/

#endif /* !defined (CELL_VARIABLE_UNEMAP_INTERFACE_H) */
