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
Global functions
----------------
*/
int file_read_graphics_objects(char *file_name,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct LIST(GT_object) *object_list);
/*******************************************************************************
LAST MODIFIED : 20 November 2002

DESCRIPTION :
==============================================================================*/

int file_read_voltex_graphics_object_from_obj(char *file_name,
	char *graphics_object_name, enum Render_type render_type,
	float time, struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Graphical_material *default_material,
	struct LIST(GT_object) *object_list);
/*******************************************************************************
LAST MODIFIED : 20 November 2002

DESCRIPTION :
==============================================================================*/
#endif
