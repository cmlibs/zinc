/*******************************************************************************
FILE : environment_map.h

LAST MODIFIED : 21 September 1998

DESCRIPTION :
The data structures used for representing environment maps.  Used by volume
textures.
==============================================================================*/
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

int file_read_Environment_map_name(FILE *file,
	struct Environment_map **environment_map_address,
	struct MANAGER(Environment_map) *environment_map_manager);
/*******************************************************************************
LAST MODIFIED : 19 June 1996

DESCRIPTION :
Reads a environment map name from a <file>.  Searchs the list of all environment
maps for one with the specified name.  If one is not found a new one is created
with the specified name and the default properties.
==============================================================================*/
#endif
