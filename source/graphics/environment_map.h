/*******************************************************************************
FILE : environment_map.h

LAST MODIFIED : 6 December 2004

DESCRIPTION :
The data structures used for representing environment maps.  Used by volume
textures.
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
#if !defined (ENVIRONMENT_MAP_H)
#define ENVIRONMENT_MAP_H

#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
#include "graphics/material.h"

/*
Global types
------------
*/
struct Environment_map
/*******************************************************************************
LAST MODIFIED : 25 July 1995

DESCRIPTION :
The properties of a environment map.
==============================================================================*/
{
	/* the name of the environment map */
	char *name;
	/* the graphical materials to use for each face of the cube */
	struct Graphical_material *face_material[6];
	/* the number of structures that point to this environment map. The
		environment map cannot be destroyed while this is greater than 0 */
	int access_count;
}; /* struct Environment_map */

DECLARE_LIST_TYPES(Environment_map);

DECLARE_MANAGER_TYPES(Environment_map);

struct Modify_environment_map_data
/*******************************************************************************
LAST MODIFIED : 22 July 1996

DESCRIPTION :
==============================================================================*/
{
	struct MANAGER(Environment_map) *environment_map_manager;
	struct MANAGER(Graphical_material) *graphical_material_manager;
}; /* struct Modify_environment_map_data */

#if defined (OLD_CODE)
/*
Global variables
----------------
*/
extern struct LIST(Environment_map) *all_environment_maps;
#endif

/*
Global functions
----------------
*/
struct Environment_map *CREATE(Environment_map)(char *name);
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Allocates memory and assigns fields for a environment map.  Adds the environment
map to the list of all environment maps.
==============================================================================*/

int DESTROY(Environment_map)(struct Environment_map **environment_map_address);
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Frees the memory for the environment map and sets <*environment_map_address> to
NULL.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Environment_map);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Environment_map);

PROTOTYPE_LIST_FUNCTIONS(Environment_map);

PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Environment_map,name,char *);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(Environment_map,name,char *);

PROTOTYPE_MANAGER_FUNCTIONS(Environment_map);

PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Environment_map,name,char *);

int modify_Environment_map(struct Parse_state *state,void *environment_map_void,
	void *modify_environment_map_data_void);
/*******************************************************************************
LAST MODIFIED : 22 July 1996

DESCRIPTION :
Modifies the properties of a environment map.  The changed values can be
specified as strings or floats but not both.
==============================================================================*/

int list_Environment_map(struct Environment_map *environment_map);
/*****************************************************************************
LAST MODIFIED : 21 September 1998

DESCRIPTION :
Write the properties of the <environment_map> to the command window.
============================================================================*/

int file_read_Environment_map_name(struct IO_stream *file,
	struct Environment_map **environment_map_address,
	struct MANAGER(Environment_map) *environment_map_manager);
/*******************************************************************************
LAST MODIFIED : 6 December 2004

DESCRIPTION :
Reads a environment map name from a <file>.  Searchs the list of all environment
maps for one with the specified name.  If one is not found a new one is created
with the specified name and the default properties.
==============================================================================*/
#endif
