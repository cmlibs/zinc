/*******************************************************************************
FILE : renderwavefront.h

LAST MODIFIED : 20 October 1998

DESCRIPTION :
Renders gtObjects to WAVEFRONT file
==============================================================================*/
#if !defined (RENDERWAVEFRONT_H)
#define RENDERWAVEFRONT_H

#include "graphics/scene.h"

/*
Global functions
----------------
*/
int export_to_wavefront(char *file_name,struct Scene *scene,
	struct Scene_object *scene_object, int full_comments);
/******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Renders the visible objects to Wavefront object files.
==============================================================================*/
#endif /* !defined (RENDERWAVEFRONT_H) */
