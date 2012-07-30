#ifndef CMISS_SCENE_VIEWER_WX_H
#define CMISS_SCENE_VIEWER_WX_H

/***************************************************************************//**
 * Creates a Cmiss_scene_viewer by creating a wxCanvas inside the specified
 * wxPanel container.If <minimum_colour_buffer_depth>,
 * <minimum_depth_buffer_depth> or <minimum_accumulation_buffer_depth> are not
 * zero then they are used to filter out the possible visuals selected for
 * graphics_buffers.  If they are zero then the accumulation_buffer_depth
 * are not tested and the maximum colour buffer depth is chosen.
 *
 * @param cmiss_scene_viewer_package  Package containing information required
 * @param parent_void  void pointer to a wxPanel in wxWidgets.
 * @param buffer_mode  see enum Cmiss_scene_viewer_buffering_mode.
 * @param stereo_mode  see enum Cmiss_scene_viewer_stereo_mode
 * @param minimum_colour_buffer_depth  minimum colour buffer depth to be set on
 *    scene viewer.
 * @param minimum_depth_buffer_depth  minimum_depth_buffer_depth to be set on
 *    scene viewer.
 * @param minimum_accumulation_buffer_depth  minimum_accumulation_buffer_depth
 *    to be set on scene viewer.
 * @return  Cmiss_scene_viewer_id if successfully create the scene viewer,
 *    otherwise NULL.
 */
Cmiss_scene_viewer_id Cmiss_scene_viewer_create_wx(
    struct Cmiss_scene_viewer_package *cmiss_scene_viewer_package,
    void *parent_void,
    enum Cmiss_scene_viewer_buffering_mode buffer_mode,
    enum Cmiss_scene_viewer_stereo_mode stereo_mode,
    int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
    int minimum_accumulation_buffer_depth);

#endif // CMISS_SCENE_VIEWER_WX_H
