/*******************************************************************************
FILE : userdef_objects.h

LAST MODIFIED : 6 December 2004

DESCRIPTION :
Prototypes for functions that access user defined graphics objects.
???RC Should be done in a different way. Objects of their own type should be
created, then passed as a void * to a CREATE(GT_userdef), along with a
destroy function and render function.
==============================================================================*/
#if !defined (USERDEF_OBJECTS_H)
#define USERDEF_OBJECTS_H

#include "general/io_stream.h"
#include "graphics/graphics_object.h"

/*
Global functions
----------------
*/
struct GT_userdef *create_robot_7dof(void);
/*******************************************************************************
LAST MODIFIED : 20 June 1996

DESCRIPTION :
Creates a 7 dof heart surgery robot.
==============================================================================*/

int file_read_userdef(struct IO_stream *file,void *userdef_address_void);
/*******************************************************************************
LAST MODIFIED : 6 December 2004

DESCRIPTION :
Reads a user defined object from a graphics object <file>. A pointer to the
newly created object is put at <*userdef_address>.
==============================================================================*/
#endif
