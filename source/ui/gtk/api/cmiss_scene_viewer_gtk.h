#ifndef CMISS_SCENE_VIEWER_GTK_H
#define CMISS_SCENE_VIEWER_GTK_H

#include <gtk/gtk.h>

Cmiss_scene_viewer_id Cmiss_scene_viewer_create_gtk(
    struct Cmiss_scene_viewer_package *cmiss_scene_viewer_package,
    GtkContainer *scene_viewer_widget,
    enum Cmiss_scene_viewer_buffering_mode buffer_mode,
    enum Cmiss_scene_viewer_stereo_mode stereo_mode,
    int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
    int minimum_accumulation_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Creates a Cmiss_scene_viewer by creating a GtkGlArea inside the specified
<scene_viewer_widget> container.
If <minimum_colour_buffer_depth>, <minimum_depth_buffer_depth> or
<minimum_accumulation_buffer_depth> are not zero then they are used to filter
out the possible visuals selected for graphics_buffers.  If they are zero then
the accumulation_buffer_depth are not tested and the maximum colour buffer depth is
chosen.
==============================================================================*/

#endif // CMISS_SCENE_VIEWER_GTK_H
