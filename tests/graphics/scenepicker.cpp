/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <zinc/status.h>
#include <zinc/core.h>
#include <zinc/context.h>
#include <zinc/field.h>
#include <zinc/fieldgroup.h>
#include <zinc/region.h>
#include <zinc/graphics.h>
#include <zinc/scene.h>
#include <zinc/scenefilter.h>
#include <zinc/sceneviewer.h>
#include <zinc/scenepicker.h>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"
#include "opencmiss/zinc/element.hpp"
#include "opencmiss/zinc/graphics.hpp"
#include "opencmiss/zinc/node.hpp"
#include "opencmiss/zinc/scenepicker.hpp"
#include "opencmiss/zinc/scene.hpp"
#include "opencmiss/zinc/sceneviewer.hpp"

TEST(cmzn_scenepicker_api, valid_args)
{
	ZincTestSetup zinc;

	cmzn_field_id field = cmzn_fieldmodule_create_field_group(zinc.fm);

	cmzn_field_group_id fieldGroup = cmzn_field_cast_group(field);

	cmzn_scenepicker_id scene_picker = cmzn_scene_create_scenepicker(zinc.scene);
	EXPECT_NE(static_cast<cmzn_scenepicker *>(0), scene_picker);

	cmzn_sceneviewermodule_id svModule = cmzn_context_get_sceneviewermodule(
		zinc.context);
	EXPECT_NE(static_cast<cmzn_sceneviewermodule *>(0), svModule);

	cmzn_sceneviewer_id sv = cmzn_sceneviewermodule_create_sceneviewer(svModule,
		CMZN_SCENEVIEWER_BUFFERING_MODE_DOUBLE, CMZN_SCENEVIEWER_STEREO_MODE_DEFAULT);
	EXPECT_NE(static_cast<cmzn_sceneviewer *>(0), sv);

	int result = cmzn_sceneviewer_set_scene(sv, zinc.scene);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_sceneviewer_set_viewport_size(sv, 512, 512);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_sceneviewer_view_all(sv);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_scenefiltermodule_id filter_module = cmzn_context_get_scenefiltermodule(zinc.context);
	EXPECT_NE(static_cast<cmzn_scenefiltermodule *>(0), filter_module);

	cmzn_scenefilter_id sf = cmzn_scenefiltermodule_create_scenefilter_graphics_type(filter_module,
		CMZN_GRAPHICS_TYPE_POINTS);
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), sf);

	cmzn_scenefiltermodule_destroy(&filter_module);

	result = cmzn_scenepicker_set_scene(scene_picker, zinc.scene);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_scenepicker_set_scenefilter(scene_picker, sf);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_scenepicker_set_sceneviewer_rectangle(scene_picker, sv,
		CMZN_SCENECOORDINATESYSTEM_WINDOW_PIXEL_TOP_LEFT, 0,
			0, 7.0, 7.0);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_element_id element = cmzn_scenepicker_get_nearest_element(scene_picker);
	EXPECT_EQ(static_cast<cmzn_element *>(0), element);

	cmzn_node_id node = cmzn_scenepicker_get_nearest_node(scene_picker);
	EXPECT_EQ(static_cast<cmzn_node *>(0), node);

	cmzn_graphics_id graphic = cmzn_scenepicker_get_nearest_element_graphics(scene_picker);
	EXPECT_EQ(static_cast<cmzn_graphics *>(0), graphic);

	graphic = cmzn_scenepicker_get_nearest_node_graphics(scene_picker);
	EXPECT_EQ(static_cast<cmzn_graphics *>(0), graphic);

	graphic = cmzn_scenepicker_get_nearest_graphics(scene_picker);
	EXPECT_EQ(static_cast<cmzn_graphics *>(0), graphic);

	result = cmzn_scenepicker_add_picked_elements_to_field_group(scene_picker, fieldGroup);
	EXPECT_EQ(static_cast<cmzn_graphics *>(0), graphic);

	result = cmzn_scenepicker_add_picked_nodes_to_field_group(scene_picker, fieldGroup);
	EXPECT_EQ(static_cast<cmzn_graphics *>(0), graphic);

	cmzn_scenefilter_destroy(&sf);

	cmzn_field_group_destroy(&fieldGroup);

	cmzn_field_destroy(&field);

	cmzn_sceneviewer_destroy(&sv);

	cmzn_sceneviewermodule_destroy(&svModule);

	cmzn_scenepicker_destroy(&scene_picker);

}

TEST(cmzn_scenepicker_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	FieldGroup fieldGroup = zinc.fm.createFieldGroup();

	Scenepicker scenePicker = zinc.scene.createScenepicker();
	EXPECT_TRUE(scenePicker.isValid());

	Sceneviewermodule svModule = zinc.context.getSceneviewermodule();
	EXPECT_TRUE(svModule.isValid());

	Sceneviewer sv = svModule.createSceneviewer(
		Sceneviewer::BUFFERING_MODE_DOUBLE, Sceneviewer::STEREO_MODE_DEFAULT);
	EXPECT_TRUE(sv.isValid());

	int result = sv.setScene(zinc.scene);
	EXPECT_EQ(CMZN_OK, result);

	result = sv.setViewportSize(512, 512);
	EXPECT_EQ(CMZN_OK, result);

	result = sv.viewAll();
	EXPECT_EQ(CMZN_OK, result);

	Scenefiltermodule sfm = zinc.context.getScenefiltermodule();
	EXPECT_TRUE(sfm.isValid());

	Scenefilter sf = sfm.createScenefilterGraphicsType(Graphics::TYPE_POINTS);
	EXPECT_TRUE(sf.isValid());

	result = scenePicker.setScene(zinc.scene);
	EXPECT_EQ(CMZN_OK, result);

	result = scenePicker.setScenefilter(sf);
	EXPECT_EQ(CMZN_OK, result);

	result = scenePicker.setSceneviewerRectangle(sv,
		SCENECOORDINATESYSTEM_WINDOW_PIXEL_TOP_LEFT, 0, 0, 7.0, 7.0);
	EXPECT_EQ(CMZN_OK, result);

	Element element = scenePicker.getNearestElement();
	EXPECT_FALSE(element.isValid());

	Node node = scenePicker.getNearestNode();
	EXPECT_FALSE(node.isValid());

	Graphics graphic = scenePicker.getNearestElementGraphics();
	EXPECT_FALSE(graphic.isValid());

	graphic = scenePicker.getNearestNodeGraphics();
	EXPECT_FALSE(graphic.isValid());

	graphic = scenePicker.getNearestGraphics();
	EXPECT_FALSE(graphic.isValid());

	result = scenePicker.addPickedElementsToFieldGroup(fieldGroup);
	EXPECT_EQ(CMZN_OK, result);

	result = scenePicker.addPickedNodesToFieldGroup(fieldGroup);
	EXPECT_EQ(CMZN_OK, result);
}
