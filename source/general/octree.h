/*******************************************************************************
FILE : octree.h

LAST MODIFIED : 20 October 2005

DESCRIPTION :
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
#if !defined (OCTREE_H)
#define OCTREE_H

#include "general/object.h"
#include "general/list.h"
#include "general/value.h"

/*
Macros
======
*/

/*
Global types
------------
*/

struct Octree;

struct Octree_object;

DECLARE_LIST_TYPES(Octree_object);

/*
Global functions
----------------
*/

PROTOTYPE_LIST_FUNCTIONS(Octree_object);

struct Octree *CREATE(Octree)(void);
/*******************************************************************************
LAST MODIFIED : 20 October 2005

DESCRIPTION :
Creates an Octree which will contain Octree_objects.
==============================================================================*/

int DESTROY(Octree)(struct Octree **octree_address);
/*******************************************************************************
LAST MODIFIED : 20 October 2005

DESCRIPTION :
Destroys an octree.
==============================================================================*/

int Octree_add_object(struct Octree *octree, struct Octree_object *object);
/*******************************************************************************
LAST MODIFIED : 20 October 2005

DESCRIPTION :
Adds the specified <object> into the <octree>
==============================================================================*/

int Octree_add_objects_near_coordinate_to_list(struct Octree *octree, 
	int dimension, FE_value *coordinate, FE_value radius, 
	struct LIST(Octree_object) *object_list);
/*******************************************************************************
LAST MODIFIED : 20 October 2005

DESCRIPTION :
Examines the <octree> for any objects closer than <radius> in an L2 norm sense
to the specified <coordinate> and adds these to the <object_list>.
==============================================================================*/

struct Octree_object *CREATE(Octree_object)(int dimension, FE_value *coordinates);
/*******************************************************************************
LAST MODIFIED : 20 October 2005

DESCRIPTION :
Creates an Octree_object having the specified <coordinates> in <dimension> space.
==============================================================================*/

int Octree_object_set_user_data(struct Octree_object *object, void *user_data);
/*******************************************************************************
LAST MODIFIED : 20 October 2005

DESCRIPTION :
Sets the <user_data> pointer associated with an octree object.
==============================================================================*/

void *Octree_object_get_user_data(struct Octree_object *object);
/*******************************************************************************
LAST MODIFIED : 20 October 2005

DESCRIPTION :
Gets the <user_data> pointer associated with an octree object.
==============================================================================*/

int DESTROY(Octree_object)(struct Octree_object **octree_address);
/*******************************************************************************
LAST MODIFIED : 20 October 2005

DESCRIPTION :
Destroys an octree.
==============================================================================*/

#endif /* !defined (OCTREE_H) */
