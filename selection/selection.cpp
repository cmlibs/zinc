/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <zinc/core.h>
#include <zinc/field.h>
#include <zinc/fieldgroup.h>
#include <zinc/fieldsubobjectgroup.h>
#include <zinc/node.h>
#include <zinc/scene.h>
#include <zinc/selection.h>

#include <zinc/field.hpp>
#include <zinc/fieldgroup.hpp>
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

void addSomeElements(cmzn_fieldmodule_id fm)
{
	cmzn_fieldmodule_begin_change(fm);
	cmzn_mesh_id mesh = cmzn_fieldmodule_find_mesh_by_dimension(fm, 3);
	EXPECT_NE(static_cast<cmzn_mesh_id>(0), mesh);
	cmzn_elementtemplate_id elementtemplate = cmzn_mesh_create_elementtemplate(mesh);
	EXPECT_NE(static_cast<cmzn_elementtemplate_id>(0), elementtemplate);
	EXPECT_EQ(CMZN_OK, cmzn_elementtemplate_set_element_shape_type(elementtemplate, CMZN_ELEMENT_SHAPE_TYPE_CUBE));
	for (int i = 1; i <= 4; ++i)
		EXPECT_EQ(CMZN_OK, cmzn_mesh_define_element(mesh, -1, elementtemplate));
	cmzn_elementtemplate_destroy(&elementtemplate);
	cmzn_mesh_destroy(&mesh);
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

	cmzn_field_id group_field = cmzn_fieldmodule_create_field_group(zinc.fm);
	cmzn_field_group_id group = cmzn_field_cast_group(group_field);
	EXPECT_NE(static_cast<cmzn_field_group_id>(0), group);
	EXPECT_EQ(CMZN_OK, result = cmzn_scene_set_selection_field(zinc.scene, group_field));
	cmzn_field_id temp = cmzn_scene_get_selection_field(zinc.scene);
	EXPECT_EQ(group_field, temp);
	cmzn_field_destroy(&temp);
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
	cmzn_field_node_group_id nodeGroup = cmzn_field_group_create_field_node_group(group, nodeset);
	cmzn_nodeset_group_id nodesetGroup = cmzn_field_node_group_get_nodeset_group(nodeGroup);
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
	cmzn_field_destroy(&group_field);
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
	cmzn_field_id group_field = cmzn_fieldmodule_create_field_group(fm);
	cmzn_fieldmodule_destroy(&fm);
	cmzn_field_group_id group = cmzn_field_cast_group(group_field);
	EXPECT_NE(static_cast<cmzn_field_group_id>(0), group);
	cmzn_scene_id scene = cmzn_region_get_scene(region);
	EXPECT_NE(static_cast<cmzn_scene_id>(0), scene);
	EXPECT_EQ(CMZN_OK, result = cmzn_scene_set_selection_field(scene, group_field));
	cmzn_field_destroy(&group_field);
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
	cmzn_field_id group_field = cmzn_fieldmodule_create_field_group(zinc.fm);
	cmzn_field_group_id group = cmzn_field_cast_group(group_field);
	EXPECT_NE(static_cast<cmzn_field_group_id>(0), group);
	cmzn_scene_id scene = cmzn_region_get_scene(zinc.root_region);
	EXPECT_NE(static_cast<cmzn_scene_id>(0), scene);
	EXPECT_EQ(CMZN_OK, result = cmzn_scene_set_selection_field(scene, group_field));
	cmzn_field_destroy(&group_field);
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

	cmzn_field_id group_field = cmzn_fieldmodule_create_field_group(zinc.fm);
	cmzn_field_group_id group = cmzn_field_cast_group(group_field);
	EXPECT_NE(static_cast<cmzn_field_group_id>(0), group);
	EXPECT_EQ(CMZN_OK, result = cmzn_field_group_add_local_region(group));
	// not the selection group yet, so no notification
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_FLAG_NONE, recordChange.changeFlags);

	// test setting as selection group when not empty
	EXPECT_EQ(CMZN_OK, result = cmzn_scene_set_selection_field(zinc.scene, group_field));
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_FLAG_ADD, recordChange.changeFlags);

	// test removing as selection group when not empty
	recordChange.changeFlags = CMZN_SELECTIONEVENT_CHANGE_FLAG_NONE;
	EXPECT_EQ(CMZN_OK, result = cmzn_scene_set_selection_field(zinc.scene, static_cast<cmzn_field_id>(0)));
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_FLAG_REMOVE, recordChange.changeFlags);

	cmzn_field_group_destroy(&group);
	cmzn_field_destroy(&group_field);
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

	EXPECT_EQ(OK, result = zinc.scene.setSelectionField(group));
	Field temp = zinc.scene.getSelectionField();
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

TEST(cmzn_fieldgroup, add_remove_nodes)
{
	ZincTestSetup zinc;
	int result;

	cmzn_region_id childRegion = cmzn_region_create_child(zinc.root_region, "child");
	cmzn_fieldmodule_id childFm = cmzn_region_get_fieldmodule(childRegion);
	addSomeNodes(childFm);
	cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(childFm, CMZN_FIELD_DOMAIN_TYPE_NODES);
	cmzn_node_id node = cmzn_nodeset_find_node_by_identifier(nodeset, 2);
	EXPECT_NE(static_cast<cmzn_node_id>(0), node);

	cmzn_field_id groupField = cmzn_fieldmodule_create_field_group(zinc.fm);
	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_name(groupField, "group"));
	cmzn_field_group_id group = cmzn_field_cast_group(groupField);
	EXPECT_NE(static_cast<cmzn_field_group_id>(0), group);

	// test find/create node group for a subregion
	cmzn_field_node_group_id nodeGroup = 0;
	EXPECT_EQ(static_cast<cmzn_field_node_group_id>(0), nodeGroup = cmzn_field_group_get_field_node_group(group, nodeset));
	EXPECT_NE(static_cast<cmzn_field_node_group_id>(0), nodeGroup = cmzn_field_group_create_field_node_group(group, nodeset));
	EXPECT_EQ(static_cast<cmzn_field_node_group_id>(0), cmzn_field_group_create_field_node_group(group, nodeset));
	char *childNodeGroupName = cmzn_field_get_name(cmzn_field_node_group_base_cast(nodeGroup));
	EXPECT_STREQ("group.nodes", childNodeGroupName);
	cmzn_deallocate(childNodeGroupName);

	// test contains/add
	cmzn_nodeset_group_id nodesetGroup = cmzn_field_node_group_get_nodeset_group(nodeGroup);
	EXPECT_NE(static_cast<cmzn_nodeset_group_id>(0), nodesetGroup);
	EXPECT_EQ(false, cmzn_nodeset_contains_node(cmzn_nodeset_group_base_cast(nodesetGroup), node));
	EXPECT_EQ(CMZN_OK, cmzn_nodeset_group_add_node(nodesetGroup, node));
	EXPECT_EQ(true, cmzn_nodeset_contains_node(cmzn_nodeset_group_base_cast(nodesetGroup), node));

	// test child group created properly, and that we find non-empty subgroup
	cmzn_field_group_id childGroup = 0;
	EXPECT_NE(static_cast<cmzn_field_group_id>(0), childGroup = cmzn_field_group_get_subregion_field_group(group, childRegion));
	char *childGroupName = cmzn_field_get_name(cmzn_field_group_base_cast(childGroup));
	EXPECT_STREQ("group", childGroupName);
	cmzn_deallocate(childGroupName);
	cmzn_field_group_id nonEmptyGroup = 0;
	EXPECT_EQ(childGroup, nonEmptyGroup = cmzn_field_group_get_first_non_empty_subregion_field_group(group));
	cmzn_field_group_destroy(&nonEmptyGroup);
	cmzn_field_node_group_id tmpNodeGroup = 0;
	EXPECT_EQ(nodeGroup, tmpNodeGroup = cmzn_field_group_get_field_node_group(childGroup, nodeset));
	cmzn_field_node_group_destroy(&tmpNodeGroup);
	cmzn_field_group_destroy(&childGroup);

	// test remove/contains
	EXPECT_EQ(CMZN_OK, cmzn_nodeset_group_remove_node(nodesetGroup, node));
	EXPECT_EQ(false, cmzn_nodeset_contains_node(cmzn_nodeset_group_base_cast(nodesetGroup), node));

	// test for empty groups
	EXPECT_EQ(static_cast<cmzn_field_group_id>(0), nonEmptyGroup = cmzn_field_group_get_first_non_empty_subregion_field_group(group));
	EXPECT_EQ(CMZN_OK, cmzn_field_group_remove_empty_subgroups(group));
	EXPECT_EQ(static_cast<cmzn_field_group_id>(0), childGroup = cmzn_field_group_get_subregion_field_group(group, childRegion));

	cmzn_nodeset_group_destroy(&nodesetGroup);
	cmzn_field_node_group_destroy(&nodeGroup);
	cmzn_field_group_destroy(&group);
	cmzn_field_destroy(&groupField);

	cmzn_node_destroy(&node);
	cmzn_nodeset_destroy(&nodeset);
	cmzn_fieldmodule_destroy(&childFm);
	cmzn_region_destroy(&childRegion);
}

TEST(ZincFieldGroup, add_remove_nodes)
{
	ZincTestSetupCpp zinc;
	int result;

	Region childRegion = zinc.root_region.createChild("child");
	Fieldmodule childFm = childRegion.getFieldmodule();
	addSomeNodes(childFm.getId());
	Nodeset nodeset = childFm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodeset.isValid());
	Node node = nodeset.findNodeByIdentifier(2);
	EXPECT_TRUE(node.isValid());

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());
	EXPECT_EQ(CMZN_OK, result = group.setName("group"));

	// test find/create node group for a subregion
	FieldNodeGroup nodeGroup;
	EXPECT_FALSE((nodeGroup = group.getFieldNodeGroup(nodeset)).isValid());
	EXPECT_TRUE((nodeGroup = group.createFieldNodeGroup(nodeset)).isValid());
	EXPECT_FALSE(group.createFieldNodeGroup(nodeset).isValid());
	char *childNodeGroupName = nodeGroup.getName();
	EXPECT_STREQ("group.nodes", childNodeGroupName);
	cmzn_deallocate(childNodeGroupName);

	// test contains/add
	NodesetGroup nodesetGroup = nodeGroup.getNodesetGroup();
	EXPECT_TRUE(nodesetGroup.isValid());
	EXPECT_FALSE(nodesetGroup.containsNode(node));
	EXPECT_EQ(OK, nodesetGroup.addNode(node));
	EXPECT_TRUE(nodesetGroup.containsNode(node));

	// test child group created properly, and that we find non-empty subgroup
	FieldGroup childGroup;
	EXPECT_TRUE((childGroup = group.getSubregionFieldGroup(childRegion)).isValid());
	char *childGroupName = childGroup.getName();
	EXPECT_STREQ("group", childGroupName);
	cmzn_deallocate(childGroupName);
	FieldGroup nonEmptyGroup;
	EXPECT_EQ(childGroup, nonEmptyGroup = group.getFirstNonEmptySubregionFieldGroup());
	FieldNodeGroup tmpNodeGroup;
	EXPECT_EQ(nodeGroup, tmpNodeGroup = childGroup.getFieldNodeGroup(nodeset));

	// test remove/contains
	EXPECT_EQ(OK, nodesetGroup.removeNode(node));
	EXPECT_FALSE(nodesetGroup.containsNode(node));

	// test for empty groups
	EXPECT_FALSE((nonEmptyGroup = group.getFirstNonEmptySubregionFieldGroup()).isValid());
	EXPECT_EQ(OK, group.removeEmptySubgroups());
	EXPECT_FALSE((childGroup = group.getSubregionFieldGroup(childRegion)).isValid());
}

TEST(cmzn_fieldgroup, add_remove_elements)
{
	ZincTestSetup zinc;
	int result;

	cmzn_region_id childRegion = cmzn_region_create_child(zinc.root_region, "child");
	cmzn_fieldmodule_id childFm = cmzn_region_get_fieldmodule(childRegion);
	addSomeElements(childFm);
	cmzn_mesh_id mesh = cmzn_fieldmodule_find_mesh_by_dimension(childFm, 3);
	cmzn_element_id element = cmzn_mesh_find_element_by_identifier(mesh, 2);
	EXPECT_NE(static_cast<cmzn_element_id>(0), element);

	cmzn_field_id groupField = cmzn_fieldmodule_create_field_group(zinc.fm);
	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_name(groupField, "group"));
	cmzn_field_group_id group = cmzn_field_cast_group(groupField);
	EXPECT_NE(static_cast<cmzn_field_group_id>(0), group);

	// test find/create element group for a subregion
	cmzn_field_element_group_id elementGroup = 0;
	EXPECT_EQ(static_cast<cmzn_field_element_group_id>(0), elementGroup = cmzn_field_group_get_field_element_group(group, mesh));
	EXPECT_NE(static_cast<cmzn_field_element_group_id>(0), elementGroup = cmzn_field_group_create_field_element_group(group, mesh));
	EXPECT_EQ(static_cast<cmzn_field_element_group_id>(0), cmzn_field_group_create_field_element_group(group, mesh));
	char *childNodeGroupName = cmzn_field_get_name(cmzn_field_element_group_base_cast(elementGroup));
	EXPECT_STREQ("group.mesh3d", childNodeGroupName);
	cmzn_deallocate(childNodeGroupName);

	// test contains/add
	cmzn_mesh_group_id meshGroup = cmzn_field_element_group_get_mesh_group(elementGroup);
	EXPECT_NE(static_cast<cmzn_mesh_group_id>(0), meshGroup);
	EXPECT_EQ(false, cmzn_mesh_contains_element(cmzn_mesh_group_base_cast(meshGroup), element));
	EXPECT_EQ(CMZN_OK, cmzn_mesh_group_add_element(meshGroup, element));
	EXPECT_EQ(true, cmzn_mesh_contains_element(cmzn_mesh_group_base_cast(meshGroup), element));

	// test child group created properly, and that we find non-empty subgroup
	cmzn_field_group_id childGroup = 0;
	EXPECT_NE(static_cast<cmzn_field_group_id>(0), childGroup = cmzn_field_group_get_subregion_field_group(group, childRegion));
	char *childGroupName = cmzn_field_get_name(cmzn_field_group_base_cast(childGroup));
	EXPECT_STREQ("group", childGroupName);
	cmzn_deallocate(childGroupName);
	cmzn_field_group_id nonEmptyGroup = 0;
	EXPECT_EQ(childGroup, nonEmptyGroup = cmzn_field_group_get_first_non_empty_subregion_field_group(group));
	cmzn_field_group_destroy(&nonEmptyGroup);
	cmzn_field_element_group_id tmpNodeGroup = 0;
	EXPECT_EQ(elementGroup, tmpNodeGroup = cmzn_field_group_get_field_element_group(childGroup, mesh));
	cmzn_field_element_group_destroy(&tmpNodeGroup);
	cmzn_field_group_destroy(&childGroup);

	// test remove/contains
	EXPECT_EQ(CMZN_OK, cmzn_mesh_group_remove_element(meshGroup, element));
	EXPECT_EQ(false, cmzn_mesh_contains_element(cmzn_mesh_group_base_cast(meshGroup), element));

	// test for empty groups
	EXPECT_EQ(static_cast<cmzn_field_group_id>(0), nonEmptyGroup = cmzn_field_group_get_first_non_empty_subregion_field_group(group));
	EXPECT_EQ(CMZN_OK, cmzn_field_group_remove_empty_subgroups(group));
	EXPECT_EQ(static_cast<cmzn_field_group_id>(0), childGroup = cmzn_field_group_get_subregion_field_group(group, childRegion));

	cmzn_mesh_group_destroy(&meshGroup);
	cmzn_field_element_group_destroy(&elementGroup);
	cmzn_field_group_destroy(&group);
	cmzn_field_destroy(&groupField);

	cmzn_element_destroy(&element);
	cmzn_mesh_destroy(&mesh);
	cmzn_fieldmodule_destroy(&childFm);
	cmzn_region_destroy(&childRegion);
}

TEST(ZincFieldGroup, add_remove_elements)
{
	ZincTestSetupCpp zinc;
	int result;

	Region childRegion = zinc.root_region.createChild("child");
	Fieldmodule childFm = childRegion.getFieldmodule();
	addSomeElements(childFm.getId());
	Mesh mesh = childFm.findMeshByDimension(3);
	EXPECT_TRUE(mesh.isValid());
	Element element = mesh.findElementByIdentifier(2);
	EXPECT_TRUE(element.isValid());

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());
	EXPECT_EQ(CMZN_OK, result = group.setName("group"));

	// test find/create element group for a subregion
	FieldElementGroup elementGroup;
	EXPECT_FALSE((elementGroup = group.getFieldElementGroup(mesh)).isValid());
	EXPECT_TRUE((elementGroup = group.createFieldElementGroup(mesh)).isValid());
	EXPECT_FALSE(group.createFieldElementGroup(mesh).isValid());
	char *childElementGroupName = elementGroup.getName();
	EXPECT_STREQ("group.mesh3d", childElementGroupName);
	cmzn_deallocate(childElementGroupName);

	// test contains/add
	MeshGroup meshGroup = elementGroup.getMeshGroup();
	EXPECT_TRUE(meshGroup.isValid());
	EXPECT_FALSE(meshGroup.containsElement(element));
	EXPECT_EQ(OK, meshGroup.addElement(element));
	EXPECT_TRUE(meshGroup.containsElement(element));

	// test child group created properly, and that we find non-empty subgroup
	FieldGroup childGroup;
	EXPECT_TRUE((childGroup = group.getSubregionFieldGroup(childRegion)).isValid());
	char *childGroupName = childGroup.getName();
	EXPECT_STREQ("group", childGroupName);
	cmzn_deallocate(childGroupName);
	FieldGroup nonEmptyGroup;
	EXPECT_EQ(childGroup, nonEmptyGroup = group.getFirstNonEmptySubregionFieldGroup());
	FieldElementGroup tmpElementGroup;
	EXPECT_EQ(elementGroup, tmpElementGroup = childGroup.getFieldElementGroup(mesh));

	// test remove/contains
	EXPECT_EQ(OK, meshGroup.removeElement(element));
	EXPECT_FALSE(meshGroup.containsElement(element));

	// test for empty groups
	EXPECT_FALSE((nonEmptyGroup = group.getFirstNonEmptySubregionFieldGroup()).isValid());
	EXPECT_EQ(OK, group.removeEmptySubgroups());
	EXPECT_FALSE((childGroup = group.getSubregionFieldGroup(childRegion)).isValid());
}
