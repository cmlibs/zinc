
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
#include "zinctestsetupcpp.hpp"
#include "zinc/element.hpp"
#include "zinc/graphic.hpp"
#include "zinc/node.hpp"
#include "zinc/scenepicker.hpp"
#include "zinc/scene.hpp"
#include "zinc/sceneviewer.hpp"

TEST(cmzn_scene_picker_api, valid_args)
{
	ZincTestSetup zinc;

	cmzn_field_id field = cmzn_fieldmodule_create_field_group(zinc.fm);

	cmzn_field_group_id fieldGroup = cmzn_field_cast_group(field);

	cmzn_scene_picker_id scene_picker = cmzn_scene_create_picker(zinc.scene);
	EXPECT_NE(static_cast<cmzn_scene_picker *>(0), scene_picker);

	cmzn_scene_viewer_module_id sv_module = cmzn_graphics_module_get_scene_viewer_module(
		zinc.gm);
	EXPECT_NE(static_cast<cmzn_scene_viewer_module *>(0), sv_module);

	cmzn_scene_viewer_id sv = cmzn_scene_viewer_module_create_scene_viewer(sv_module,
		CMZN_SCENE_VIEWER_BUFFERING_DOUBLE, CMZN_SCENE_VIEWER_STEREO_ANY_MODE);
	EXPECT_NE(static_cast<cmzn_scene_viewer *>(0), sv);

	int result = cmzn_scene_viewer_set_scene(sv, zinc.scene);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_scene_viewer_set_viewport_size(sv, 512, 512);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_scene_viewer_view_all(sv);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_graphics_filter_module_id filter_module = cmzn_graphics_module_get_filter_module(zinc.gm);
	EXPECT_NE(static_cast<cmzn_graphics_filter_module *>(0), filter_module);

	cmzn_graphics_filter_id gf = cmzn_graphics_filter_module_create_filter_graphic_type(filter_module,
		CMZN_GRAPHIC_POINTS);
	EXPECT_NE(static_cast<cmzn_graphics_filter *>(0), gf);

	cmzn_graphics_filter_module_destroy(&filter_module);

	result = cmzn_scene_picker_set_scene(scene_picker, zinc.scene);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_scene_picker_set_graphics_filter(scene_picker, gf);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_scene_picker_set_scene_viewer_rectangle(scene_picker, sv,
		CMZN_SCENE_COORDINATE_SYSTEM_WINDOW_PIXEL_TOP_LEFT, 0,
			0, 7.0, 7.0);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_element_id element = cmzn_scene_picker_get_nearest_element(scene_picker);
	EXPECT_EQ(static_cast<cmzn_element *>(0), element);

	cmzn_node_id node = cmzn_scene_picker_get_nearest_node(scene_picker);
	EXPECT_EQ(static_cast<cmzn_node *>(0), node);

	cmzn_graphic_id graphic = cmzn_scene_picker_get_nearest_element_graphic(scene_picker);
	EXPECT_EQ(static_cast<cmzn_graphic *>(0), graphic);

	graphic = cmzn_scene_picker_get_nearest_node_graphic(scene_picker);
	EXPECT_EQ(static_cast<cmzn_graphic *>(0), graphic);

	graphic = cmzn_scene_picker_get_nearest_graphic(scene_picker);
	EXPECT_EQ(static_cast<cmzn_graphic *>(0), graphic);

	result = cmzn_scene_picker_add_picked_elements_to_group(scene_picker, fieldGroup);
	EXPECT_EQ(static_cast<cmzn_graphic *>(0), graphic);

	result = cmzn_scene_picker_add_picked_nodes_to_group(scene_picker, fieldGroup);
	EXPECT_EQ(static_cast<cmzn_graphic *>(0), graphic);

	cmzn_graphics_filter_destroy(&gf);

	cmzn_field_group_destroy(&fieldGroup);

	cmzn_field_destroy(&field);

	cmzn_scene_viewer_destroy(&sv);

	cmzn_scene_viewer_module_destroy(&sv_module);

	cmzn_scene_picker_destroy(&scene_picker);

}

TEST(cmzn_scene_picker_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	FieldGroup fieldGroup = zinc.fm.createFieldGroup();

	ScenePicker scenePicker = zinc.scene.createPicker();
	EXPECT_TRUE(scenePicker.isValid());

	SceneViewerModule sv_module = zinc.gm.getSceneViewerModule();
	EXPECT_TRUE(sv_module.isValid());

	SceneViewer sv = sv_module.createSceneViewer(
		SceneViewer::BUFFERING_MODE_DOUBLE, SceneViewer::STEREO_MODE_ANY);
	EXPECT_TRUE(sv.isValid());

	int result = sv.setScene(zinc.scene);
	EXPECT_EQ(CMZN_OK, result);

	result = sv.setViewportSize(512, 512);
	EXPECT_EQ(CMZN_OK, result);

	result = sv.viewAll();
	EXPECT_EQ(CMZN_OK, result);

	GraphicsFilterModule gfm = zinc.gm.getFilterModule();
	EXPECT_TRUE(gfm.isValid());

	GraphicsFilter gf = gfm.createFilterGraphicType(Graphic::POINTS);
	EXPECT_TRUE(gf.isValid());

	result = scenePicker.setScene(zinc.scene);
	EXPECT_EQ(CMZN_OK, result);

	result = scenePicker.setGraphicsFilter(gf);
	EXPECT_EQ(CMZN_OK, result);

	result = scenePicker.setSceneViewerRectangle(sv,
		SCENE_COORDINATE_SYSTEM_WINDOW_PIXEL_TOP_LEFT, 0, 0, 7.0, 7.0);
	EXPECT_EQ(CMZN_OK, result);

	Element element = scenePicker.getNearestElement();
	EXPECT_FALSE(element.isValid());

	Node node = scenePicker.getNearestNode();
	EXPECT_FALSE(node.isValid());

	Graphic graphic = scenePicker.getNearestElementGraphic();
	EXPECT_FALSE(graphic.isValid());

	graphic = scenePicker.getNearestNodeGraphic();
	EXPECT_FALSE(graphic.isValid());

	graphic = scenePicker.getNearestGraphic();
	EXPECT_FALSE(graphic.isValid());

	result = scenePicker.addPickedElementsToGroup(fieldGroup);
	EXPECT_EQ(CMZN_OK, result);

	result = scenePicker.addPickedNodesToGroup(fieldGroup);
	EXPECT_EQ(CMZN_OK, result);
}
