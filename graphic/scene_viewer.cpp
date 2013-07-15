#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/status.h>
#include <zinc/core.h>
#include <zinc/context.h>
#include <zinc/sceneviewer.h>

TEST(Cmiss_scene_viewer_api, destroy_context_before_scene_viewer)
{
     Cmiss_context_id context = Cmiss_context_create("test");
     Cmiss_graphics_module_id gm = Cmiss_context_get_graphics_module(context);
     Cmiss_scene_viewer_module_id scene_viewer_module = Cmiss_graphics_module_get_scene_viewer_module(gm);
     Cmiss_scene_viewer_id scene_viewer = Cmiss_scene_viewer_module_create_scene_viewer(scene_viewer_module,
   	  CMISS_SCENE_VIEWER_BUFFERING_ANY_MODE, CMISS_SCENE_VIEWER_STEREO_ANY_MODE);
     Cmiss_scene_viewer_module_destroy(&scene_viewer_module);
     Cmiss_graphics_module_destroy(&gm);
     Cmiss_context_destroy(&context);
     Cmiss_scene_viewer_destroy(&scene_viewer);
}

