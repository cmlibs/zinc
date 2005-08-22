/*******************************************************************************
FILE : cell_variable.h

LAST MODIFIED : 15 March 2001

DESCRIPTION :
Routines for using the Cell_variable objects
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
#if !defined (CELL_VARIABLE_H)
#define CELL_VARIABLE_H

#include "general/object.h"
#include "general/list.h"

#include "cell/cell_types.h"

/*
Module objects
--------------
*/
struct Cell_variable;
/*******************************************************************************
LAST MODIFIED : 17 January 2001

DESCRIPTION :
A data object used to store information for all variables found in the CellML.
==============================================================================*/
DECLARE_LIST_TYPES(Cell_variable);

/*
Global functions
----------------
*/
PROTOTYPE_OBJECT_FUNCTIONS(Cell_variable);
PROTOTYPE_LIST_FUNCTIONS(Cell_variable);
/*PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Cell_variable,id,int);*/
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Cell_variable,name,char *);

struct Cell_variable *CREATE(Cell_variable)(char *name);
/*******************************************************************************
LAST MODIFIED : 02 July 2000

DESCRIPTION :
Creates a Cell_variable object.
==============================================================================*/
int DESTROY(Cell_variable)(struct Cell_variable **cell_variable_address);
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Destroys a Cell_variable object.
==============================================================================*/
int Cell_variable_set_name(struct Cell_variable *cell_variable,
  char *name);
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
Sets the <name> of the <cell_variable> - copies the <name> so the calling
routine should deallocate it.
==============================================================================*/
int Cell_variable_set_display_name(struct Cell_variable *cell_variable,
  char *display_name);
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
Sets the <display_name> of the <cell_variable> - copies the <display_name> so
the calling routine should deallocate it.
==============================================================================*/
int Cell_variable_set_value_type(struct Cell_variable *cell_variable,
  enum Cell_value_type value_type);
/*******************************************************************************
LAST MODIFIED : 20 October 2000

DESCRIPTION :
Sets the value type of the <cell_variable> to be <value_type>.
==============================================================================*/
int Cell_variable_set_value_from_string(struct Cell_variable *cell_variable,
  char *value_string);
/*******************************************************************************
LAST MODIFIED : 20 October 2000

DESCRIPTION :
Sets the value of the <cell_variable> from the given <value_string>
==============================================================================*/
char *Cell_variable_get_value_as_string(struct Cell_variable *cell_variable);
/*******************************************************************************
LAST MODIFIED : 09 November 2000

DESCRIPTION :
Returns the value of the <cell_variable> as a string.
==============================================================================*/
int Cell_variable_list(struct Cell_variable *cell_variable,void *full_void);
/*******************************************************************************
LAST MODIFIED : 09 July 2000

DESCRIPTION :
Iterator function used to list out the current variables. If <full> is not 0,
then a full listing of the <cell_variable> is given, otherwise just a brief
listing.
==============================================================================*/
int Cell_variable_set_cmiss_interface(struct Cell_variable *cell_variable,
  char *array_string,char *position_string);
/*******************************************************************************
LAST MODIFIED : 25 October 2000

DESCRIPTION :
Sets the CMISS variable interface information for the given <cell_variable>
==============================================================================*/
struct Cell_cmiss_interface *Cell_variable_get_cmiss_interface(
  struct Cell_variable *cell_variable);
/*******************************************************************************
LAST MODIFIED : 28 October 2000

DESCRIPTION :
Gets the CMISS variable interface for the given <cell_variable>
==============================================================================*/
struct Cell_variable_unemap_interface *Cell_variable_get_unemap_interface(
  struct Cell_variable *cell_variable);
/*******************************************************************************
LAST MODIFIED : 03 November 2000

DESCRIPTION :
Gets the UnEmap variable interface for the given <cell_variable>
==============================================================================*/
CELL_REAL Cell_variable_get_real_value(struct Cell_variable *cell_variable);
/*******************************************************************************
LAST MODIFIED : 28 October 2000

DESCRIPTION :
Returns the variable's value as a real.
==============================================================================*/
CELL_INTEGER Cell_variable_get_integer_value(
  struct Cell_variable *cell_variable);
/*******************************************************************************
LAST MODIFIED : 28 October 2000

DESCRIPTION :
Returns the variable's value as a integer.
==============================================================================*/
int Cell_variable_set_unemap_interface(struct Cell_variable *cell_variable);
/*******************************************************************************
LAST MODIFIED : 01 November 2000

DESCRIPTION :
Sets the UnEmap variable interface information for the given <cell_variable>
==============================================================================*/
char *Cell_variable_get_name(struct Cell_variable *cell_variable);
/*******************************************************************************
LAST MODIFIED : 08 November 2000

DESCRIPTION :
Returns a copy of the name for the given <cell_variable>
==============================================================================*/
char *Cell_variable_get_display_name(struct Cell_variable *cell_variable);
/*******************************************************************************
LAST MODIFIED : 08 November 2000

DESCRIPTION :
Returns a copy of the display name for the given <cell_variable>
==============================================================================*/
int Cell_variable_variable_has_unemap_interface(
  struct Cell_variable *cell_variable);
/*******************************************************************************
LAST MODIFIED : 09 November 2000

DESCRIPTION :
Returns 1 if the given <cell_variable> has a UnEmap interface defined.
==============================================================================*/
int Cell_variable_variable_has_cmiss_interface(
  struct Cell_variable *cell_variable);
/*******************************************************************************
LAST MODIFIED : 14 March 2001

DESCRIPTION :
Returns 1 if the given <cell_variable> has a CMISS interface defined.
==============================================================================*/
int Cell_variable_destroy_unemap_interface(struct Cell_variable *cell_variable);
/*******************************************************************************
LAST MODIFIED : 09 November 2000

DESCRIPTION :
If the <cell_variable> has a UnEmap interface, it is destroyed.
==============================================================================*/
int Cell_variable_set_changed(struct Cell_variable *cell_variable,int value);
/*******************************************************************************
LAST MODIFIED : 09 November 2000

DESCRIPTION :
Sets the changed field of the given <cell_variable> object to <value>.
==============================================================================*/
int Cell_variable_get_changed(struct Cell_variable *cell_variable);
/*******************************************************************************
LAST MODIFIED : 09 November 2000

DESCRIPTION :
Gets the changed field of the given <cell_variable>.
==============================================================================*/
char *Cell_variable_check_value_string(struct Cell_variable *cell_variable,
  char *value_string);
/*******************************************************************************
LAST MODIFIED : 09 November 2000

DESCRIPTION :
Checks the <value_string> for a valid value for the given <cell_variable> and
returns the value in "Cell format" if a valid value is found, otherwise a
NULL string is returned.
==============================================================================*/
int Cell_variable_add_value_text_field(struct Cell_variable *variable,
  void *widget_void);
/*******************************************************************************
LAST MODIFIED : 18 January 2000

DESCRIPTION :
Adds the given text field <widget> to the <variable>'s list of value widgets.
==============================================================================*/
int Cell_variable_delete_value_text_fields(struct Cell_variable *variable,
  void *user_data_void);
/*******************************************************************************
LAST MODIFIED : 19 January 2000

DESCRIPTION :
Iterator function used to delete all value text field widgets for the given
<variable>.
==============================================================================*/
int Cell_variable_set_ode(struct Cell_variable *cell_variable);
/*******************************************************************************
LAST MODIFIED : 15 March 2001

DESCRIPTION :
Sets the ODE field of the given <cell_variable>'s CMISS interface object to be
true.
==============================================================================*/

#endif /* !defined (CELL_VARIABLE_H) */
