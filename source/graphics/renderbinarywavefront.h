/*******************************************************************************
FILE : renderbinarywavefront.h

LAST MODIFIED : 20 October 1998

DESCRIPTION :
Renders gtObjects to BINARY WAVEFRONT file
==============================================================================*/
#if !defined (RENDERBINARYWAVEFRONT_H)
#define RENDERBINARYWAVEFRONT_H

#include "graphics/scene.h"

/*
Global functions
----------------
*/
int export_to_binary_wavefront(char *file_name,
	struct Scene_object *scene_object, int number_of_frames, int version,
	int frame_number );
/******************************************************************************
LAST MODIFIED : 8 June 1999

DESCRIPTION :
Renders the visible objects to binary Wavefront object files.
==============================================================================*/
#endif /* !defined (RENDERWAVEFRONT_H) */
