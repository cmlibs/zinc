#ifndef CMISS_SCENE_VIEWER_WIN32_H
#define CMISS_SCENE_VIEWER_WIN32_H

#if !defined (NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>

Cmiss_scene_viewer_id Cmiss_scene_viewer_create_win32(
    struct Cmiss_scene_viewer_package *cmiss_scene_viewer_package,
    HWND hWnd, HDC hDC,
    enum Cmiss_scene_viewer_buffering_mode buffer_mode,
    enum Cmiss_scene_viewer_stereo_mode stereo_mode,
    int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
    int minimum_accumulation_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 1 June 2007

DESCRIPTION :
Creates a Cmiss_scene_viewer by creating a graphics buffer on the specified
<hWnd> window handle.  If the <hDC> is specified it is used to render.
Alternatively if <hWnd> is NULL and <hDC> is specified then no window functions
are performed but the graphics window will render into the supplied device context.
If <minimum_colour_buffer_depth>, <minimum_depth_buffer_depth> or
<minimum_accumulation_buffer_depth> are not zero then they are used to filter
out the possible visuals selected for graphics_buffers.  If they are zero then
the accumulation_buffer_depth are not tested and the maximum colour buffer depth is
chosen.
==============================================================================*/

int Cmiss_scene_viewer_handle_windows_event(Cmiss_scene_viewer_id scene_viewer,
    UINT event,WPARAM first_message,LPARAM second_message);
/*******************************************************************************
LAST MODIFIED : 31 May 2007

DESCRIPTION:
Passes the supplied windows event on to the graphics buffer.
==============================================================================*/

int Cmiss_scene_viewer_win32_set_window_size(Cmiss_scene_viewer_id scene_viewer,
    int width, int height, int x, int y);
/*******************************************************************************
LAST MODIFIED : 14 September 2007

DESCRIPTION :
Sets the maximum extent of the graphics window within which individual paints
will be requested with handle_windows_event.
==============================================================================*/

#endif // CMISS_SCENE_VIEWER_WIN32_H
