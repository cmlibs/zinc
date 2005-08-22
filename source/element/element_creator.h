/*******************************************************************************
FILE : element_creator.h

LAST MODIFIED : 17 January 2003

DESCRIPTION :
Dialog for choosing the type of element constructed in response to node
selections. Elements are created in this way while dialog is open.
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
#if !defined (ELEMENT_CREATOR_H)
#define ELEMENT_CREATOR_H

#include "finite_element/finite_element.h"
#include "graphics/scene.h"
#include "selection/element_selection.h"
#include "selection/node_selection.h"

/*
Global types
------------
*/

struct Element_creator;

/*
Global functions
----------------
*/

struct Element_creator *CREATE(Element_creator)(
	struct Element_creator **element_creator_address,
	struct Cmiss_region *root_region, char *initial_region_path,
	struct FE_element_selection *element_selection,
	struct FE_node_selection *node_selection,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 14 January 2003

DESCRIPTION :
Creates a Element_creator, giving it the element_manager to put new
elements in, and the node_manager for new nodes, and the fe_field_manager to
enable the creation of a coordinate field.
==============================================================================*/

int DESTROY(Element_creator)(
	struct Element_creator **element_creator_address);
/*******************************************************************************
LAST MODIFIED : 9 May 2000

DESCRIPTION :
Deaccesses objects and frees memory used by the Element_creator at
<*element_creator_address>.
==============================================================================*/

struct FE_field *Element_creator_get_coordinate_field(
	struct Element_creator *element_creator);
/*******************************************************************************
LAST MODIFIED : 17 May 2000

DESCRIPTION :
Returns the coordinate field interpolated by elements created with
<element_creator>.
==============================================================================*/

int Element_creator_set_coordinate_field(
	struct Element_creator *element_creator,struct FE_field *coordinate_field);
/*******************************************************************************
LAST MODIFIED : 17 May 2000

DESCRIPTION :
Sets the coordinate field interpolated by elements created with
<element_creator>.
==============================================================================*/

int Element_creator_get_create_enabled(struct Element_creator *element_creator);
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
Returns flag controlling whether node edits are updated during motion_notify
events, not just at the end of a mouse gesture.
==============================================================================*/

int Element_creator_set_create_enabled(struct Element_creator *element_creator,
	int create_enabled);
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
Sets flag controlling whether elements are created in response to
node selection.
==============================================================================*/

int Element_creator_get_element_dimension(
	struct Element_creator *element_creator);
/*******************************************************************************
LAST MODIFIED : 9 May 2000

DESCRIPTION :
Returns the dimension of elements to be created by the <element_creator>.
==============================================================================*/

int Element_creator_set_element_dimension(
	struct Element_creator *element_creator,int element_dimension);
/*******************************************************************************
LAST MODIFIED : 9 May 2000

DESCRIPTION :
Sets the <element_dimension> of any subsequent elements to be created.
==============================================================================*/

int Element_creator_get_region_path(struct Element_creator *element_creator,
	char **path_address);
/*******************************************************************************
LAST MODIFIED : 10 January 2003

DESCRIPTION :
Returns in <path_address> the path to the Cmiss_region where elements created by
the <element_creator> are put.
Up to the calling function to DEALLOCATE the returned path.
==============================================================================*/

int Element_creator_set_region_path(struct Element_creator *element_creator,
	char *path);
/*******************************************************************************
LAST MODIFIED : 13 January 2003

DESCRIPTION :
Sets the <path> to the region/FE_region where elements created by
<element_creator> are placed.
The <element_creator> assumes its nodes are to come from the same FE_region.
==============================================================================*/

int Element_creator_bring_window_to_front(
	struct Element_creator *element_creator);
/*******************************************************************************
LAST MODIFIED : 9 May 2000

DESCRIPTION :
Pops the window for <element_creator> to the front of those visible.
??? De-iconify as well?
==============================================================================*/

#endif /* !defined (ELEMENT_CREATOR_H) */


