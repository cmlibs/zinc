/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <zinc/field.h>
#include <zinc/fieldgroup.h>
#include <zinc/fieldmodule.h>
#include <zinc/fieldsubobjectgroup.h>
#include <zinc/node.h>
#include <zinc/scene.h>
#include <zinc/selection.h>

#include <zinc/field.hpp>
#include <zinc/fieldgroup.hpp>
#include <zinc/fieldmodule.hpp>
#include <zinc/fieldsubobjectgroup.hpp>
#include <zinc/node.hpp>
#include <zinc/scene.hpp>
#include <zinc/selection.hpp>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

struct RecordChange
{
	int changeFlags;
};

void selectionCallback(cmzn_selectionevent_id selectionevent,
	void *recordChangeVoid)
{
	RecordChange *recordChange = reinterpret_cast<RecordChange*>(recordChangeVoid);
	recordChange->changeFlags = cmzn_selectionevent_get_change_flags(selectionevent);
}

void addSomeNodes(cmzn_fieldmodule_id fm)
{
	cmzn_fieldmodule_begin_change(fm);
	cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(fm, CMZN_FIELD_DOMAIN_TYPE_NODES);
	EXPECT_NE(static_cast<cmzn_nodeset_id>(0), nodeset);
	cmzn_nodetemplate_id nodetemplate = cmzn_nodeset_create_nodetemplate(nodeset);
	EXPECT_NE(static_cast<cmzn_nodetemplate_id>(0), nodetemplate);
	int result;
	for (int i = 1; i <= 4; ++i)
	{
		cmzn_node_id node = cmzn_nodeset_create_node(nodeset, -1, nodetemplate);
		EXPECT_NE(static_cast<cmzn_node_id>(0), node);
		EXPECT_EQ(CMZN_OK, result = cmzn_node_destroy(&node));
	}
	cmzn_nodetemplate_destroy(&nodetemplate);
	cmzn_nodeset_destroy(&nodeset);
	cmzn_fieldmodule_end_change(fm);
}

TEST(cmzn_selectionnotifier, changeCallback)
{
	ZincTestSetup zinc;
	int result;

	cmzn_selectionnotifier_id selectionnotifier = cmzn_scene_create_selectionnotifier(zinc.scene);
	EXPECT_NE(static_cast<cmzn_selectionnotifier_id>(0), selectionnotifier);

	RecordChange recordChange;
	recordChange.changeFlags = CMZN_SELECTIONEVENT_CHANGE_FLAG_NONE;
	EXPECT_EQ(CMZN_OK, result = cmzn_selectionnotifier_set_callback(selectionnotifier,
		selectionCallback, static_cast<void*>(&recordChange)));

	cmzn_field_id field = cmzn_fieldmodule_create_field_group(zinc.fm);
	cmzn_field_group_id group = cmzn_field_cast_group(field);
	cmzn_field_destroy(&field);
	EXPECT_NE(static_cast<cmzn_field_group_id>(0), group);
	EXPECT_EQ(CMZN_OK, result = cmzn_scene_set_selection_group(zinc.scene, group));
	cmzn_field_group_id temp = cmzn_scene_get_selection_group(zinc.scene);
	EXPECT_EQ(temp, group);
	cmzn_field_group_destroy(&temp);
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_FLAG_NONE, recordChange.changeFlags);
	EXPECT_FALSE(cmzn_field_group_contains_local_region(group));

	EXPECT_EQ(CMZN_OK, result = cmzn_field_group_add_local_region(group));
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_FLAG_ADD, recordChange.changeFlags);
	EXPECT_TRUE(cmzn_field_group_contains_local_region(group));

	EXPECT_EQ(CMZN_OK, result = cmzn_field_group_remove_local_region(group));
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_FLAG_REMOVE, recordChange.changeFlags);
	EXPECT_FALSE(cmzn_field_group_contains_local_region(group));

	addSomeNodes(zinc.fm);

	cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(zinc.fm, CMZN_FIELD_DOMAIN_TYPE_NODES);
	cmzn_field_node_group_id nodeGroup = cmzn_field_group_create_node_group(group, nodeset);
	cmzn_nodeset_group_id nodesetGroup = cmzn_field_node_group_get_nodeset(nodeGroup);
	cmzn_node_id node1 = cmzn_nodeset_find_node_by_identifier(nodeset, 1);
	EXPECT_NE(static_cast<cmzn_node_id>(0), node1);
	cmzn_node_id node2 = cmzn_nodeset_find_node_by_identifier(nodeset, 2);
	EXPECT_NE(static_cast<cmzn_node_id>(0), node2);
	EXPECT_FALSE(cmzn_nodeset_contains_node(cmzn_nodeset_group_base_cast(nodesetGroup), node1));
	EXPECT_EQ(CMZN_OK, result = cmzn_nodeset_group_add_node(nodesetGroup, node1));
	EXPECT_TRUE(cmzn_nodeset_contains_node(cmzn_nodeset_group_base_cast(nodesetGroup), node1));
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_FLAG_ADD, recordChange.changeFlags);

	recordChange.changeFlags = CMZN_SELECTIONEVENT_CHANGE_FLAG_NONE;
	EXPECT_NE(CMZN_OK, result = cmzn_nodeset_group_add_node(nodesetGroup, node1)); // already in group
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_FLAG_NONE, recordChange.changeFlags);

	recordChange.changeFlags = CMZN_SELECTIONEVENT_CHANGE_FLAG_NONE;
	EXPECT_NE(CMZN_OK, cmzn_nodeset_group_remove_node(nodesetGroup, node2)); // not in group
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_FLAG_NONE, recordChange.changeFlags);

	EXPECT_FALSE(cmzn_nodeset_contains_node(cmzn_nodeset_group_base_cast(nodesetGroup), node2));
	EXPECT_EQ(CMZN_OK, result = cmzn_nodeset_group_add_node(nodesetGroup, node2));
	EXPECT_TRUE(cmzn_nodeset_contains_node(cmzn_nodeset_group_base_cast(nodesetGroup), node2));
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_FLAG_ADD, recordChange.changeFlags);

	EXPECT_EQ(CMZN_OK, result = cmzn_nodeset_group_remove_node(nodesetGroup, node2));
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_FLAG_REMOVE, recordChange.changeFlags);

	EXPECT_EQ(CMZN_OK, result = cmzn_nodeset_group_remove_node(nodesetGroup, node1));
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_FLAG_REMOVE, recordChange.changeFlags);

	EXPECT_EQ(CMZN_OK, result = cmzn_nodeset_group_add_node(nodesetGroup, node1));
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_FLAG_ADD, recordChange.changeFlags);

	cmzn_fieldmodule_begin_change(zinc.fm);
	EXPECT_EQ(CMZN_OK, result = cmzn_nodeset_group_remove_node(nodesetGroup, node1));
	EXPECT_EQ(CMZN_OK, result = cmzn_nodeset_group_add_node(nodesetGroup, node2));
	cmzn_fieldmodule_end_change(zinc.fm);
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_FLAG_ADD | CMZN_SELECTIONEVENT_CHANGE_FLAG_REMOVE, recordChange.changeFlags);

	recordChange.changeFlags = CMZN_SELECTIONEVENT_CHANGE_FLAG_NONE;
	cmzn_fieldmodule_begin_change(zinc.fm);
	EXPECT_EQ(CMZN_OK, result = cmzn_nodeset_group_add_node(nodesetGroup, node1));
	EXPECT_EQ(CMZN_OK, result = cmzn_nodeset_group_remove_node(nodesetGroup, node2));
	cmzn_fieldmodule_end_change(zinc.fm);
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_FLAG_ADD | CMZN_SELECTIONEVENT_CHANGE_FLAG_REMOVE, recordChange.changeFlags);

	EXPECT_EQ(CMZN_OK, result = cmzn_field_group_add_local_region(group));
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_FLAG_ADD, recordChange.changeFlags);

	recordChange.changeFlags = CMZN_SELECTIONEVENT_CHANGE_FLAG_NONE;
	EXPECT_EQ(CMZN_OK, result = cmzn_field_group_add_local_region(group));
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_FLAG_NONE, recordChange.changeFlags);

	EXPECT_EQ(CMZN_OK, result = cmzn_field_group_clear_local(group));
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_FLAG_REMOVE, recordChange.changeFlags);

	cmzn_node_destroy(&node1);
	cmzn_node_destroy(&node2);

	EXPECT_EQ(CMZN_OK, result = cmzn_selectionnotifier_clear_callback(selectionnotifier));

	cmzn_nodeset_group_destroy(&nodesetGroup);
	cmzn_field_node_group_destroy(&nodeGroup);
	cmzn_nodeset_destroy(&nodeset);
	cmzn_field_group_destroy(&group);
	EXPECT_EQ(CMZN_OK, result = cmzn_selectionnotifier_destroy(&selectionnotifier));
}

// check selection callbacks safely handle destruction of scene
TEST(cmzn_selectionnotifier, destroy_scene)
{
	ZincTestSetup zinc;
	int result;

	cmzn_region_id region = cmzn_region_create_child(zinc.root_region, "child");
	EXPECT_NE(static_cast<cmzn_region_id>(0), region);
	cmzn_fieldmodule_id fm = cmzn_region_get_fieldmodule(region);
	cmzn_field_id field = cmzn_fieldmodule_create_field_group(fm);
	cmzn_fieldmodule_destroy(&fm);
	cmzn_field_group_id group = cmzn_field_cast_group(field);
	EXPECT_NE(static_cast<cmzn_field_group_id>(0), group);
	cmzn_field_destroy(&field);
	cmzn_scene_id scene = cmzn_region_get_scene(region);
	EXPECT_NE(static_cast<cmzn_scene_id>(0), scene);
	EXPECT_EQ(CMZN_OK, result = cmzn_scene_set_selection_group(scene, group));
	cmzn_selectionnotifier_id selectionnotifier = cmzn_scene_create_selectionnotifier(scene);
	EXPECT_NE(static_cast<cmzn_selectionnotifier_id>(0), selectionnotifier);
	cmzn_scene_destroy(&scene);

	RecordChange recordChange;
	recordChange.changeFlags = CMZN_SELECTIONEVENT_CHANGE_FLAG_NONE;
	EXPECT_EQ(CMZN_OK, result = cmzn_selectionnotifier_set_callback(selectionnotifier,
		selectionCallback, static_cast<void*>(&recordChange)));

	EXPECT_EQ(CMZN_OK, result = cmzn_field_group_add_local_region(group));
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_FLAG_ADD, recordChange.changeFlags);

	cmzn_region_remove_child(zinc.root_region, region);
	cmzn_region_destroy(&region);

	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_FLAG_FINAL, recordChange.changeFlags);

	EXPECT_EQ(CMZN_OK, result = cmzn_selectionnotifier_clear_callback(selectionnotifier));
	EXPECT_EQ(CMZN_OK, result = cmzn_selectionnotifier_destroy(&selectionnotifier));
}

// test that group changes from child regions are propagated to parent selection
TEST(cmzn_selectionnotifier, hierarchical_change)
{
	ZincTestSetup zinc;
	int result;

	cmzn_region_id childRegion = cmzn_region_create_child(zinc.root_region, "child");
	EXPECT_NE(static_cast<cmzn_region_id>(0), childRegion);
	cmzn_field_id field = cmzn_fieldmodule_create_field_group(zinc.fm);
	cmzn_field_group_id group = cmzn_field_cast_group(field);
	EXPECT_NE(static_cast<cmzn_field_group_id>(0), group);
	cmzn_field_destroy(&field);
	cmzn_scene_id scene = cmzn_region_get_scene(zinc.root_region);
	EXPECT_NE(static_cast<cmzn_scene_id>(0), scene);
	EXPECT_EQ(CMZN_OK, result = cmzn_scene_set_selection_group(scene, group));
	cmzn_selectionnotifier_id selectionnotifier = cmzn_scene_create_selectionnotifier(scene);
	EXPECT_NE(static_cast<cmzn_selectionnotifier_id>(0), selectionnotifier);
	cmzn_scene_destroy(&scene);

	RecordChange recordChange;
	recordChange.changeFlags = CMZN_SELECTIONEVENT_CHANGE_FLAG_NONE;
	EXPECT_EQ(CMZN_OK, result = cmzn_selectionnotifier_set_callback(selectionnotifier,
		selectionCallback, static_cast<void*>(&recordChange)));

	EXPECT_EQ(CMZN_OK, result = cmzn_field_group_add_region(group, childRegion));
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_FLAG_ADD, recordChange.changeFlags);
	EXPECT_TRUE(cmzn_field_group_contains_region(group, childRegion));

	EXPECT_EQ(CMZN_OK, result = cmzn_field_group_remove_region(group, childRegion));
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_FLAG_REMOVE, recordChange.changeFlags);
	EXPECT_FALSE(cmzn_field_group_contains_region(group, childRegion));

	// test removal of non-empty childRegion
	EXPECT_EQ(CMZN_OK, result = cmzn_field_group_add_region(group, childRegion));
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_FLAG_ADD, recordChange.changeFlags);
	EXPECT_TRUE(cmzn_field_group_contains_region(group, childRegion));

	recordChange.changeFlags = CMZN_SELECTIONEVENT_CHANGE_FLAG_NONE;
	cmzn_region_remove_child(zinc.root_region, childRegion);
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_FLAG_REMOVE, recordChange.changeFlags);
	EXPECT_FALSE(cmzn_field_group_contains_region(group, childRegion));
	cmzn_region_destroy(&childRegion);

	EXPECT_EQ(CMZN_OK, result = cmzn_selectionnotifier_destroy(&selectionnotifier));
}

// test that removing selection group reports REMOVE
TEST(cmzn_selectionnotifier, add_remove_selection_group)
{
	ZincTestSetup zinc;
	int result;

	cmzn_selectionnotifier_id selectionnotifier = cmzn_scene_create_selectionnotifier(zinc.scene);
	EXPECT_NE(static_cast<cmzn_selectionnotifier_id>(0), selectionnotifier);

	RecordChange recordChange;
	recordChange.changeFlags = CMZN_SELECTIONEVENT_CHANGE_FLAG_NONE;
	EXPECT_EQ(CMZN_OK, result = cmzn_selectionnotifier_set_callback(selectionnotifier,
		selectionCallback, static_cast<void*>(&recordChange)));

	cmzn_field_id field = cmzn_fieldmodule_create_field_group(zinc.fm);
	cmzn_field_group_id group = cmzn_field_cast_group(field);
	cmzn_field_destroy(&field);
	EXPECT_NE(static_cast<cmzn_field_group_id>(0), group);
	EXPECT_EQ(CMZN_OK, result = cmzn_field_group_add_local_region(group));
	// not the selection group yet, so no notification
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_FLAG_NONE, recordChange.changeFlags);

	// test setting as selection group when not empty
	EXPECT_EQ(CMZN_OK, result = cmzn_scene_set_selection_group(zinc.scene, group));
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_FLAG_ADD, recordChange.changeFlags);

	// test removing as selection group when not empty
	recordChange.changeFlags = CMZN_SELECTIONEVENT_CHANGE_FLAG_NONE;
	EXPECT_EQ(CMZN_OK, result = cmzn_scene_set_selection_group(zinc.scene, static_cast<cmzn_field_group_id>(0)));
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_FLAG_REMOVE, recordChange.changeFlags);

	cmzn_field_group_destroy(&group);
	EXPECT_EQ(CMZN_OK, result = cmzn_selectionnotifier_destroy(&selectionnotifier));
}

class SelectioncallbackRecordChange : public Selectioncallback
{
	int changeFlags;

	virtual void operator()(const Selectionevent &selectionevent)
	{
		this->changeFlags = selectionevent.getChangeFlags();
	}

public:
	SelectioncallbackRecordChange() :
			Selectioncallback(),
			changeFlags(Selectionevent::CHANGE_FLAG_NONE)
	{ }

	void clear()
	{
		changeFlags = Selectionevent::CHANGE_FLAG_NONE;
	}

	int getChangeSummary() const
	{
		return this->changeFlags;
	}

};

TEST(ZincSelectionnotifier, changeCallback)
{
	ZincTestSetupCpp zinc;
	int result;

	Selectionnotifier selectionnotifier = zinc.scene.createSelectionnotifier();
	EXPECT_TRUE(selectionnotifier.isValid());

	SelectioncallbackRecordChange callback;
	EXPECT_EQ(OK, result = selectionnotifier.setCallback(callback));

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());

	// test casting
	FieldGroup castGroup = group.castGroup();
	EXPECT_TRUE(castGroup.isValid());

	EXPECT_EQ(OK, result = zinc.scene.setSelectionGroup(group));
	FieldGroup temp = zinc.scene.getSelectionGroup();
	EXPECT_EQ(group, temp);
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_NONE, callback.getChangeSummary());
	EXPECT_FALSE(group.containsLocalRegion());

	EXPECT_EQ(CMZN_OK, result = group.addLocalRegion());
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_ADD, callback.getChangeSummary());
	EXPECT_TRUE(group.containsLocalRegion());

	EXPECT_EQ(CMZN_OK, result = group.removeLocalRegion());
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_REMOVE, callback.getChangeSummary());
	EXPECT_FALSE(group.containsLocalRegion());

	EXPECT_EQ(OK, result = selectionnotifier.clearCallback());
}
