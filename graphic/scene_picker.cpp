
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/status.h>
#include <zinc/core.h>
#include <zinc/context.h>
#include <zinc/field.h>
#include <zinc/fieldgroup.h>
#include <zinc/region.h>
#include <zinc/graphic.h>
#include <zinc/graphicsfilter.h>
#include <zinc/scene.h>
#include <zinc/sceneviewer.h>
#include <zinc/scenepicker.h>

#include "zinctestsetup.hpp"

TEST(Cmiss_scene_picker_api, valid_args)
{
	ZincTestSetup zinc;

	Cmiss_field_id field = Cmiss_field_module_create_group(zinc.fm);

	Cmiss_field_group_id fieldGroup = Cmiss_field_cast_group(field);

	Cmiss_scene_id scene = Cmiss_graphics_module_create_scene(zinc.gm);
	EXPECT_NE(static_cast<Cmiss_scene *>(0), scene);

	Cmiss_scene_picker_id scene_picker = Cmiss_scene_create_picker(scene);
	EXPECT_NE(static_cast<Cmiss_scene_picker *>(0), scene_picker);

	Cmiss_scene_viewer_package_id sv_package = Cmiss_context_get_default_scene_viewer_package(
		zinc.context);
	EXPECT_NE(static_cast<Cmiss_scene_viewer_package *>(0), sv_package);

	Cmiss_scene_viewer_id sv = Cmiss_scene_viewer_package_create_scene_viewer(sv_package,
		CMISS_SCENE_VIEWER_BUFFERING_DOUBLE, CMISS_SCENE_VIEWER_STEREO_ANY_MODE);
	EXPECT_NE(static_cast<Cmiss_scene_viewer *>(0), sv);

	int result = Cmiss_scene_viewer_set_scene(sv, scene);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_scene_viewer_set_viewport_size(sv, 512, 512);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_scene_viewer_view_all(sv);
	EXPECT_EQ(CMISS_OK, result);

	Cmiss_graphics_filter_id gf = Cmiss_graphics_module_create_filter_graphic_type(zinc.gm,
		CMISS_GRAPHIC_NODE_POINTS);
	EXPECT_NE(static_cast<Cmiss_graphics_filter *>(0), gf);

	result = Cmiss_scene_picker_set_scene(scene_picker, scene);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_scene_picker_set_graphics_filter(scene_picker, gf);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_scene_picker_set_scene_viewer_rectangle(scene_picker, sv,
		CMISS_GRAPHICS_COORDINATE_SYSTEM_WINDOW_PIXEL_TOP_LEFT, 0,
			0, 7.0, 7.0);
	EXPECT_EQ(CMISS_OK, result);

	Cmiss_element_id element = Cmiss_scene_picker_get_nearest_element(scene_picker);
	EXPECT_EQ(static_cast<Cmiss_element *>(0), element);

	Cmiss_node_id node = Cmiss_scene_picker_get_nearest_node(scene_picker);
	EXPECT_EQ(static_cast<Cmiss_node *>(0), node);

	Cmiss_graphic_id graphic = Cmiss_scene_picker_get_nearest_element_graphic(scene_picker);
	EXPECT_EQ(static_cast<Cmiss_graphic *>(0), graphic);

	graphic = Cmiss_scene_picker_get_nearest_node_graphic(scene_picker);
	EXPECT_EQ(static_cast<Cmiss_graphic *>(0), graphic);

	graphic = Cmiss_scene_picker_get_nearest_graphic(scene_picker);
	EXPECT_EQ(static_cast<Cmiss_graphic *>(0), graphic);

	result = Cmiss_scene_picker_add_picked_elements_to_group(scene_picker, fieldGroup);
	EXPECT_EQ(static_cast<Cmiss_graphic *>(0), graphic);

	result = Cmiss_scene_picker_add_picked_nodes_to_group(scene_picker, fieldGroup);
	EXPECT_EQ(static_cast<Cmiss_graphic *>(0), graphic);

	Cmiss_field_group_destroy(&fieldGroup);

	Cmiss_field_destroy(&field);

	Cmiss_scene_viewer_destroy(&sv);

	Cmiss_scene_viewer_package_destroy(&sv_package);

	Cmiss_scene_picker_destroy(&scene_picker);

	Cmiss_scene_destroy(&scene);
}
