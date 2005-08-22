/*******************************************************************************
FILE : cell_graphic.h

LAST MODIFIED : 20 November 2000

DESCRIPTION :
Routines for using the Cell_graphic objects
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
#if !defined (CELL_GRAPHIC_H)
#define CELL_GRAPHIC_H

#include "cell/cell_cmgui_interface.h"
#include "general/list.h"

/*
Module objects
--------------
*/
struct Cell_graphic;
/*******************************************************************************
LAST MODIFIED : 18 November 2000

DESCRIPTION :
A data object used to store information about the graphics used to represent
a Cell_component object in the Cell 3D scene.
==============================================================================*/
DECLARE_LIST_TYPES(Cell_graphic);

/*
Global functions
----------------
*/
PROTOTYPE_OBJECT_FUNCTIONS(Cell_graphic);
PROTOTYPE_LIST_FUNCTIONS(Cell_graphic);
/*PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Cell_component,id,int);*/
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Cell_graphic,name,char *);

struct Cell_graphic *CREATE(Cell_graphic)(char *name);
/*******************************************************************************
LAST MODIFIED : 18 November 2000

DESCRIPTION :
Creates a Cell_graphic object.
==============================================================================*/
int DESTROY(Cell_graphic)(struct Cell_graphic **cell_graphic_address);
/*******************************************************************************
LAST MODIFIED : 18 November 2000

DESCRIPTION :
Destroys a Cell_graphic object.
==============================================================================*/
int Cell_graphic_create_graphical_object(struct Cell_graphic *cell_graphic,
  struct Cell_cmgui_interface *cmgui_interface,char *obj_file);
/*******************************************************************************
LAST MODIFIED : 18 November 2000

DESCRIPTION :
Creates the <cell_graphic>'s graphical object. If <obj_file> is non-NULL, then
that OBJ file is used to create the graphical object - otherwise a sime arrow
is created.
==============================================================================*/
int Cell_graphic_create_graphical_material(struct Cell_graphic *cell_graphic,
  struct Cell_cmgui_interface *cmgui_interface,
  char *diffuse_red,char *diffuse_green,char *diffuse_blue,
  char *ambient_red,char *ambient_green,char *ambient_blue,
  char *emission_red,char *emission_green,char *emission_blue,
  char *specular_red,char *specular_green,char *specular_blue,
  char *alpha,char *shininess);
/*******************************************************************************
LAST MODIFIED : 18 November 2000

DESCRIPTION :
Creates the material for the given <cell_graphic> and assigns it to the
graphic's graphical object.
==============================================================================*/
struct GT_object *Cell_graphic_get_graphics_object(
  struct Cell_graphic *cell_graphic);
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Returns the graphics object for the given <cell_graphic>
==============================================================================*/

#endif /* !defined (CELL_GRAPHIC_H) */
