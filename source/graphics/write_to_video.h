/*******************************************************************************
FILE : write_to_video.h

LAST MODIFIED : 4 December 1994

DESCRIPTION :
Function prototypes for writing images through a socket to another process which
will in turn write them to video.
???DB.  Should use cmiss/Jack's socket routines ?
==============================================================================*/
#if !defined (WRITE_TO_VIDEO_H)
#define WRITE_TO_VIDEO_H

#include <stddef.h>
#include "graphics/graphics_window.h"

/*
Global functions
----------------
*/
int set_video_on_off(struct Graphics_window *graphics);
/*******************************************************************************
LAST MODIFIED : 5 October 1993

DESCRIPTION :
Toggles writing to video on and off.
==============================================================================*/

int grab_frame(int number_of_frames);
/*******************************************************************************
LAST MODIFIED : 4 December 1993

DESCRIPTION :
Grabs a frame from the screen and writes it <number_of_frames> times to the
video recorder.
==============================================================================*/
#endif
