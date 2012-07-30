#ifndef CMISS_SCENE_VIEWER_CARBON_H
#define CMISS_SCENE_VIEWER_CARBON_H

#include <carbon/carbon.h>

Cmiss_scene_viewer_id Cmiss_scene_viewer_create_Carbon(
    struct Cmiss_scene_viewer_package *cmiss_scene_viewer_package,
    WindowRef windowIn,
    enum Cmiss_scene_viewer_buffering_mode buffer_mode,
    enum Cmiss_scene_viewer_stereo_mode stereo_mode,
    int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
    int minimum_accumulation_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 27 November 2006

DESCRIPTION :
Creates a Cmiss_scene_viewer by creating a graphics buffer on the specified
<port> window handle.
If <minimum_colour_buffer_depth>, <minimum_depth_buffer_depth> or
<minimum_accumulation_buffer_depth> are not zero then they are used to filter
out the possible visuals selected for graphics_buffers.  If they are zero then
the accumulation_buffer_depth are not tested and the maximum colour buffer depth is
chosen.
==============================================================================*/

int Cmiss_scene_viewer_carbon_set_window_size(Cmiss_scene_viewer_id scene_viewer,
    int width, int height, int clip_width, int clip_height);
/*******************************************************************************
LAST MODIFIED : 16 February 2007

DESCRIPTION :
Sets the coordinates within the graphics port which the scene_viewer should
respect.
==============================================================================*/

#endif // CMISS_SCENE_VIEWER_CARBON_H
