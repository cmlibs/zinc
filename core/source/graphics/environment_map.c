/*******************************************************************************
FILE : environment_map.c

LAST MODIFIED : 6 December 2004

DESCRIPTION :
The functions for manipulating environment maps.
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
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "general/debug.h"
#include "general/list_private.h"
#include "general/io_stream.h"
#include "general/manager_private.h"
#include "general/mystring.h"
#include "general/object.h"
#include "graphics/environment_map.h"
#include "general/message.h"

/*
Module types
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
	const char *name;
	/* the graphical materials to use for each face of the cube */
	struct Graphical_material *face_material[6];

	/* after clearing in create, following to be modified only by manager */
	struct MANAGER(Environment_map) *manager;
	int manager_change_status;

	/* the number of structures that point to this environment map. The
		environment map cannot be destroyed while this is greater than 0 */
	int access_count;
}; /* struct Environment_map */

FULL_DECLARE_LIST_TYPE(Environment_map);

FULL_DECLARE_MANAGER_TYPE(Environment_map);

/*
Module functions
----------------
*/

DECLARE_LOCAL_MANAGER_FUNCTIONS(Environment_map)

/*
Global functions
----------------
*/
struct Environment_map *CREATE(Environment_map)(const char *name)
/*******************************************************************************
LAST MODIFIED : 1 September 1996

DESCRIPTION :
Allocates memory and assigns fields for a environment map.  Adds the environment
map to the list of all environment maps.
==============================================================================*/
{
	int i;
	struct Environment_map *environment_map;
	char *temp_name

	ENTER(CREATE(Environment_map));
	/* allocate memory for structure */
	if (ALLOCATE(environment_map,struct Environment_map,1))
	{
		if (name)
		{
			if (ALLOCATE(temp_name,char,strlen(name)+1))
			{
				strcpy(temp_name,name);
				environment_map->name = temp_name;
			}
		}
		else
		{
			if (ALLOCATE(temp_name,char,1))
			{
				*(temp_name)='\0';
				environment_map->name = temp_name;
			}
		}
		if (environment_map->name)
		{
			environment_map->access_count=0;
			environment_map->manager = (struct MANAGER(Environment_map) *)NULL;
			environment_map->manager_change_status = MANAGER_CHANGE_NONE(Environment_map);
			for (i=5;i>=0;i--)
			{
				(environment_map->face_material)[i]=
					(struct Graphical_material *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Environment_map).  Insufficient memory for name");
			DEALLOCATE(environment_map);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Environment_map).  Insufficient memory for structure");
	}
	LEAVE;

	return (environment_map);
} /* CREATE(Environment_map) */

int DESTROY(Environment_map)(struct Environment_map **environment_map_address)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Frees the memory for the environment map and sets <*environment_map_address> to
NULL.
==============================================================================*/
{
	int i,return_code;
	struct Environment_map *environment_map;

	ENTER(DESTROY(Environment_map));
	if (environment_map_address)
	{
		environment_map= *environment_map_address;
		if (environment_map)
		{
			if (environment_map->access_count<=0)
			{
				DEALLOCATE(environment_map->name);
				for (i=5;i>=0;i--)
				{
					DEACCESS(Graphical_material)(&((environment_map->face_material)[i]));
				}
				DEALLOCATE(*environment_map_address);
				return_code=1;
			}
			else
			{
				return_code=0;
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Environment_map).  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Environment_map) */

DECLARE_OBJECT_FUNCTIONS(Environment_map)
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Environment_map)

DECLARE_LIST_FUNCTIONS(Environment_map)

DECLARE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Environment_map,name,const char *,strcmp)
DECLARE_LIST_IDENTIFIER_CHANGE_FUNCTIONS(Environment_map,name)

int list_Environment_map(struct Environment_map *environment_map)
/*******************************************************************************
LAST MODIFIED : 25 July 1995

DESCRIPTION :
Writes the properties of the <environment_map> to the command window.
==============================================================================*/
{
	char line[80];
	int face_no,return_code;

	ENTER(list_Environment_map);
	/* check the arguments */
	if (environment_map)
	{
		display_message(INFORMATION_MESSAGE,"environment map : ");
		display_message(INFORMATION_MESSAGE,environment_map->name);
		display_message(INFORMATION_MESSAGE,"\n");
		for (face_no=0;face_no<6;face_no++)
		{
			if ((environment_map->face_material)[face_no])
			{
				sprintf(line,"  face %d : %s\n",face_no,
					Graphical_material_name(environment_map->face_material[face_no]));
			}
			else
			{
				sprintf(line,"  face %d : no material\n",face_no);
			}
			display_message(INFORMATION_MESSAGE,line);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"list_Environment_map.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_Environment_map */

PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(Environment_map,name)
{
	char *name;
	int return_code;

	ENTER(MANAGER_COPY_WITH_IDENTIFIER(Environment_map,name));
	/* check arguments */
	if (source&&destination)
	{
		if (source->name)
		{
			if (ALLOCATE(name,char,strlen(source->name)+1))
			{
				strcpy(name,source->name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(Environment_map,name).  Insufficient memory");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			return_code = MANAGER_COPY_WITHOUT_IDENTIFIER(Environment_map,name)(
				destination, source);
			if (return_code)
			{
				/* copy values */
				DEALLOCATE(destination->name);
				destination->name=name;
			}
			else
			{
				DEALLOCATE(name);
				display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(Environment_map,name).  Could not copy without identifier");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(Environment_map,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITH_IDENTIFIER(Environment_map,name) */

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(Environment_map,name)
{
	int return_code,i;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Environment_map,name));
	/* check arguments */
	if (source&&destination)
	{
		/* copy values */
		for (i=0;i<6;i++)
		{
			REACCESS(Graphical_material)(&(destination->face_material[i]),
				source->face_material[i]);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(Environment_map,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITHOUT_IDENTIFIER(Environment_map,name) */

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(Environment_map,name,const char *)
{
	char *destination_name = NULL;
	int return_code = 0;

	ENTER(MANAGER_COPY_IDENTIFIER(Environment_map,name));
	/* check arguments */
	if (name&&destination)
	{
		if (name)
		{
			if (ALLOCATE(destination_name,char,strlen(name)+1))
			{
				strcpy(destination_name,name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(Environment_map,name).  Insufficient memory");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			/* copy name */
			DEALLOCATE(destination->name);
			destination->name=destination_name;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(Environment_map,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_IDENTIFIER(Environment_map,name) */

DECLARE_MANAGER_FUNCTIONS(Environment_map,manager)

DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Environment_map,manager)

DECLARE_MANAGER_IDENTIFIER_FUNCTIONS(Environment_map,name,const char *,manager)

int file_read_Environment_map_name(struct IO_stream *stream,
	struct Environment_map **environment_map_address,
	struct MANAGER(Environment_map) *environment_map_manager)
/*******************************************************************************
LAST MODIFIED : 6 December 2004

DESCRIPTION :
Reads a environment map name from a <file>.  Searchs the list of all environment
maps for one with the specified name.  If one is not found a new one is created
with the specified name and the default properties.
==============================================================================*/
{
	char *environment_map_name;
	int return_code;
	struct Environment_map *environment_map;

	ENTER(file_read_Environment_map_name);
	/* check the arguments */
	if (stream&&environment_map_address)
	{
		if (IO_stream_read_string(stream,"s",&environment_map_name))
		{
			/*???DB.  Should this read function be in another module ? */
			environment_map=FIND_BY_IDENTIFIER_IN_MANAGER(Environment_map,name)(
				environment_map_name,environment_map_manager);
			if (environment_map)
			{
				*environment_map_address=environment_map;
				return_code=1;
			}
			else
			{
				environment_map=CREATE(Environment_map)(environment_map_name);
				if (environment_map)
				{
					if (ADD_OBJECT_TO_MANAGER(Environment_map)(environment_map,
						environment_map_manager))
					{
						*environment_map_address=environment_map;
						return_code=1;
					}
					else
					{
						DESTROY(Environment_map)(&environment_map);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
					"file_read_Environment_map_name.  Could not create environment map");
					return_code=0;
				}
			}
			DEALLOCATE(environment_map_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
	"file_read_Environment_map_name.  Error reading environment map name strin");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"file_read_Environment_map_name.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* file_read_Environment_map_name */
