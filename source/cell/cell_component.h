/*******************************************************************************
FILE : cell_component.h

LAST MODIFIED : 20 November 2000

DESCRIPTION :
Routines for using the Cell_component objects
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
#if !defined (CELL_COMPONENT_H)
#define CELL_COMPONENT_H

#include "cell/cell_graphic.h"
#include "cell/cell_variable.h"

#include "general/list.h"
#include "general/any_object_prototype.h"

/*
Module objects
--------------
*/
struct Cell_component;
/*******************************************************************************
LAST MODIFIED : 02 July 2000

DESCRIPTION :
A data object used to store information about any object from the CellML which
the user may decide to display and interact with in the Cell interface.
==============================================================================*/
DECLARE_LIST_TYPES(Cell_component);

/*
Global functions
----------------
*/
PROTOTYPE_ANY_OBJECT(Cell_component);

PROTOTYPE_OBJECT_FUNCTIONS(Cell_component);
PROTOTYPE_LIST_FUNCTIONS(Cell_component);
/*PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Cell_component,id,int);*/
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Cell_component,name,char *);

struct Cell_component *CREATE(Cell_component)(struct Cell_component *parent,
  char *name);
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
Creates a Cell_component object.
==============================================================================*/
int DESTROY(Cell_component)(struct Cell_component **cell_component_address);
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Destroys a Cell_component object.
==============================================================================*/
int Cell_component_list(struct Cell_component *cell_component,void *full_void);
/*******************************************************************************
LAST MODIFIED : 09 July 2000

DESCRIPTION :
Iterator function used to list out the current components. If <full> is not 0,
then a full listing of the <cell_component> is given, otherwise just a brief
listing.
==============================================================================*/
int Cell_component_destroy_variable_list(struct Cell_component *cell_component,
  void *unused_void);
/*******************************************************************************
LAST MODIFIED : 09 July 2000

DESCRIPTION :
Iterator function used to remove all the variable references in the variable
list of the <cell_component>
==============================================================================*/
int Cell_component_set_name(struct Cell_component *cell_component,
  char *name);
/*******************************************************************************
LAST MODIFIED : 09 July 2000

DESCRIPTION :
Sets the <name> of the <cell_component> - copies the <name> so the calling
routine should deallocate it.
==============================================================================*/
int Cell_component_set_display_name(struct Cell_component *cell_component,
  char *display_name);
/*******************************************************************************
LAST MODIFIED : 09 July 2000

DESCRIPTION :
Sets the <display_name> of the <cell_component> - copies the <display_name>
so the calling routine should deallocate it.
==============================================================================*/
int Cell_component_add_variable_to_variable_list(
  struct Cell_component *cell_component,
  struct Cell_variable *cell_variable);
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
Adds the <cell_variable> to the <cell_component>'s variable list.
==============================================================================*/
int Cell_component_add_variable_to_exported_variable_list(
  struct Cell_component *cell_component,
  struct Cell_variable *cell_variable);
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Adds the <cell_variable> to the <cell_component>'s exported variable list.
==============================================================================*/
int Cell_component_add_variable_to_imported_variable_list(
  struct Cell_component *cell_component,
  struct Cell_variable *cell_variable);
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Adds the <cell_variable> to the <cell_component>'s imported variable list.
Creates the list if this is the first variable to be added to the list.
==============================================================================*/
int Cell_component_add_child_to_children_list(
  struct Cell_component *cell_component,
  struct Cell_component *child);
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Adds the <child> to the <cell_component>'s list of children
==============================================================================*/
int Cell_component_list_component_hierarchy(
  struct Cell_component *cell_component,void *indent_level_void);
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Lists out the given <cell_component>'s hierarchy.
==============================================================================*/
struct Cell_variable *Cell_component_get_cell_variable_by_name(
  struct Cell_component *cell_component,char *name);
/*******************************************************************************
LAST MODIFIED : 25 October 2000

DESCRIPTION :
Returns the cell variable given by <name> from the <cell_component>'s variable
list - or NULL if a coresponding variable is not found.
==============================================================================*/
char *Cell_component_get_name(struct Cell_component *cell_component);
/*******************************************************************************
LAST MODIFIED : 25 October 2000

DESCRIPTION :
Returns a copy of the <cell_component>'s name.
==============================================================================*/
char *Cell_component_get_display_name(struct Cell_component *cell_component);
/*******************************************************************************
LAST MODIFIED : 25 October 2000

DESCRIPTION :
Returns a copy of the <cell_component>'s display name.
==============================================================================*/
int Cell_component_component_has_children(
  struct Cell_component *cell_component);
/*******************************************************************************
LAST MODIFIED : 07 November 2000

DESCRIPTION :
Returns 1 if the <cell_component> has any children components, otherwise 0.
==============================================================================*/
struct LIST(Cell_component) *Cell_component_get_children_list(
  struct Cell_component *cell_component);
/*******************************************************************************
LAST MODIFIED : 07 November 2000

DESCRIPTION :
Returns the children list of the given <cell_component>
==============================================================================*/
struct LIST(Cell_variable) *Cell_component_get_variable_list(
  struct Cell_component *cell_component);
/*******************************************************************************
LAST MODIFIED : 08 November 2000

DESCRIPTION :
Returns the variable list of the given <cell_component>
==============================================================================*/
int Cell_component_set_graphical_transformation(
  struct Cell_component *component,char *pos_x,char *pos_y,char *pos_z,
  char *dir_x,char *dir_y,char *dir_z,char *scale_x,char *scale_y,
  char *scale_z);
/*******************************************************************************
LAST MODIFIED : 18 November 2000

DESCRIPTION :
Sets the transformation to be used for the graphical representation of the
given <component>
==============================================================================*/
int Cell_component_set_graphic(struct Cell_component *cell_component,
  struct Cell_graphic *cell_graphic);
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Sets the <cell_graphic> for the given <cell_component>
==============================================================================*/
struct Cell_graphic *Cell_component_get_graphic(
  struct Cell_component *cell_component);
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Gets the Cell graphic for the given <cell_component>
==============================================================================*/
FE_value Cell_component_get_axis1(struct Cell_component *component,int index);
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Returns the value of axis1[<index>]
==============================================================================*/
FE_value Cell_component_get_axis2(struct Cell_component *component,int index);
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Returns the value of axis2[<index>]
==============================================================================*/
FE_value Cell_component_get_axis3(struct Cell_component *component,int index);
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Returns the value of axis3[<index>]
==============================================================================*/
FE_value Cell_component_get_point(struct Cell_component *component,int index);
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Returns the value of point[<index>]
==============================================================================*/

#endif /* !defined (CELL_COMPONENT_H) */
