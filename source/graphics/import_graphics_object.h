/*******************************************************************************
FILE : import_graphics_object.h

LAST MODIFIED : 20 June 1996

DESCRIPTION :
Function prototype for reading graphics object data from a file.
==============================================================================*/
#if !defined (IMPORT_GRAPHICS_OBJECT_H)
#define IMPORT_GRAPHICS_OBJECT_H

#include "graphics/graphics_object.h"
#include "graphics/material.h"

/*
Global types
------------
*/
struct File_read_graphics_object_data
/*******************************************************************************
LAST MODIFIED : 20 June 1996

DESCRIPTION :
Data needed by file_read_graphics_objects.
==============================================================================*/
{
	struct MANAGER(Graphical_material) *graphical_material_manager;
	struct LIST(GT_object) *object_list;
}; /* File_read_graphics_object_data */

struct File_read_graphics_object_from_obj_data
/*******************************************************************************
LAST MODIFIED : 3 February 1999

DESCRIPTION :
Data needed by file_read_voltex_graphics_object_from_obj.
==============================================================================*/
{
	char *graphics_object_name;
	enum Render_type render_type;
	float time;
	struct MANAGER(Graphical_material) *graphical_material_manager;
	struct LIST(GT_object) *object_list;
}; /* File_read_graphics_object_from_obj_data */

/*
Global functions
----------------
*/
int file_read_graphics_objects(char *file_name,
	void *file_read_graphics_object_data_void);
/*******************************************************************************
LAST MODIFIED : 20 June 1996

DESCRIPTION :
==============================================================================*/

int file_read_voltex_graphics_object_from_obj(char *file_name,
	void *file_read_graphics_object_from_obj_data_void);
/*******************************************************************************
LAST MODIFIED : 3 February 1999

DESCRIPTION :
==============================================================================*/
#endif
