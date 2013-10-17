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
	cmzn_selectionevent_change_type changeType;
};

void selectionCallback(cmzn_selectionevent_id selectionevent,
	void *recordChangeVoid)
{
	RecordChange *recordChange = reinterpret_cast<RecordChange*>(recordChangeVoid);
	recordChange->changeType = cmzn_selectionevent_get_change_type(selectionevent);
}

void addSomeNodes(cmzn_fieldmodule_id fm)
{
	cmzn_fieldmodule_begin_change(fm);
	cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_domain_type(fm, CMZN_FIELD_DOMAIN_NODES);
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

	EXPECT_FALSE(cmzn_selectionnotifier_is_hierarchical(selectionnotifier));
	EXPECT_EQ(CMZN_OK, result = cmzn_selectionnotifier_set_hierarchical(selectionnotifier, true));
	EXPECT_TRUE(cmzn_selectionnotifier_is_hierarchical(selectionnotifier));

	RecordChange recordChange;
	recordChange.changeType = CMZN_SELECTIONEVENT_CHANGE_NONE;
	EXPECT_EQ(CMZN_OK, result = cmzn_selectionnotifier_set_callback(selectionnotifier,
		selectionCallback, static_cast<void*>(&recordChange)));

	cmzn_field_id field = cmzn_fieldmodule_create_field_group(zinc.fm);
	cmzn_field_group_id group = cmzn_field_cast_group(field);
	EXPECT_NE(static_cast<cmzn_field_group_id>(0), group);
	EXPECT_EQ(CMZN_OK, result = cmzn_scene_set_selection_group(zinc.scene, group));
	cmzn_field_group_id temp = cmzn_scene_get_selection_group(zinc.scene);
	EXPECT_EQ(temp, group);
	cmzn_field_group_destroy(&temp);
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_NONE, recordChange.changeType);
	EXPECT_FALSE(cmzn_field_group_contains_local_region(group));

	EXPECT_EQ(CMZN_OK, result = cmzn_field_group_add_local_region(group));
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_ADD, recordChange.changeType);
	EXPECT_TRUE(cmzn_field_group_contains_local_region(group));

	EXPECT_EQ(CMZN_OK, result = cmzn_field_group_remove_local_region(group));
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_CLEAR, recordChange.changeType);
	EXPECT_FALSE(cmzn_field_group_contains_local_region(group));

	addSomeNodes(zinc.fm);

	cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_domain_type(zinc.fm, CMZN_FIELD_DOMAIN_NODES);
	cmzn_field_node_group_id nodeGroup = cmzn_field_group_create_node_group(group, nodeset);
	cmzn_nodeset_group_id nodesetGroup = cmzn_field_node_group_get_nodeset(nodeGroup);
	cmzn_node_id node1 = cmzn_nodeset_find_node_by_identifier(nodeset, 1);
	EXPECT_NE(static_cast<cmzn_node_id>(0), node1);
	cmzn_node_id node2 = cmzn_nodeset_find_node_by_identifier(nodeset, 2);
	EXPECT_NE(static_cast<cmzn_node_id>(0), node2);
	EXPECT_FALSE(cmzn_nodeset_contains_node(cmzn_nodeset_group_base_cast(nodesetGroup), node1));
	EXPECT_EQ(CMZN_OK, result = cmzn_nodeset_group_add_node(nodesetGroup, node1));
	EXPECT_TRUE(cmzn_nodeset_contains_node(cmzn_nodeset_group_base_cast(nodesetGroup), node1));
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_ADD, recordChange.changeType);

	recordChange.changeType = CMZN_SELECTIONEVENT_CHANGE_NONE;
	EXPECT_NE(CMZN_OK, result = cmzn_nodeset_group_add_node(nodesetGroup, node1)); // already in group
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_NONE, recordChange.changeType);

	recordChange.changeType = CMZN_SELECTIONEVENT_CHANGE_NONE;
	EXPECT_NE(CMZN_OK, cmzn_nodeset_group_remove_node(nodesetGroup, node2)); // not in group
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_NONE, recordChange.changeType);

	EXPECT_FALSE(cmzn_nodeset_contains_node(cmzn_nodeset_group_base_cast(nodesetGroup), node2));
	EXPECT_EQ(CMZN_OK, result = cmzn_nodeset_group_add_node(nodesetGroup, node2));
	EXPECT_TRUE(cmzn_nodeset_contains_node(cmzn_nodeset_group_base_cast(nodesetGroup), node2));
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_ADD, recordChange.changeType);

	EXPECT_EQ(CMZN_OK, result = cmzn_nodeset_group_remove_node(nodesetGroup, node2));
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_REMOVE, recordChange.changeType);

	EXPECT_EQ(CMZN_OK, result = cmzn_nodeset_group_remove_node(nodesetGroup, node1));
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_CLEAR, recordChange.changeType);

	EXPECT_EQ(CMZN_OK, result = cmzn_nodeset_group_add_node(nodesetGroup, node1));
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_ADD, recordChange.changeType);

	cmzn_fieldmodule_begin_change(zinc.fm);
	EXPECT_EQ(CMZN_OK, result = cmzn_nodeset_group_remove_node(nodesetGroup, node1));
	EXPECT_EQ(CMZN_OK, result = cmzn_nodeset_group_add_node(nodesetGroup, node2));
	cmzn_fieldmodule_end_change(zinc.fm);
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_REPLACE, recordChange.changeType);

	recordChange.changeType = CMZN_SELECTIONEVENT_CHANGE_NONE;
	cmzn_fieldmodule_begin_change(zinc.fm);
	EXPECT_EQ(CMZN_OK, result = cmzn_nodeset_group_add_node(nodesetGroup, node1));
	EXPECT_EQ(CMZN_OK, result = cmzn_nodeset_group_remove_node(nodesetGroup, node2));
	cmzn_fieldmodule_end_change(zinc.fm);
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_REPLACE, recordChange.changeType);

	EXPECT_EQ(CMZN_OK, result = cmzn_field_group_add_local_region(group));
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_ADD, recordChange.changeType);

	recordChange.changeType = CMZN_SELECTIONEVENT_CHANGE_NONE;
	EXPECT_EQ(CMZN_OK, result = cmzn_field_group_add_local_region(group));
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_NONE, recordChange.changeType);

	EXPECT_EQ(CMZN_OK, result = cmzn_field_group_clear_local(group));
	EXPECT_EQ(CMZN_SELECTIONEVENT_CHANGE_CLEAR, recordChange.changeType);

	cmzn_node_destroy(&node1);
	cmzn_node_destroy(&node2);

	EXPECT_EQ(CMZN_OK, result = cmzn_selectionnotifier_clear_callback(selectionnotifier));

	cmzn_nodeset_group_destroy(&nodesetGroup);
	cmzn_field_node_group_destroy(&nodeGroup);
	cmzn_nodeset_destroy(&nodeset);
	cmzn_field_group_destroy(&group);
	cmzn_field_destroy(&field);
	EXPECT_EQ(CMZN_OK, result = cmzn_selectionnotifier_destroy(&selectionnotifier));
}

// check selection callbacks safely handle destruction of scene
TEST(cmzn_selectionnotifier, destroy_scene)
{
	ZincTestSetup zinc;
	int result;

	cmzn_region_id region = cmzn_region_create_child(zinc.root_region, "child");
	EXPECT_NE(static_cast<cmzn_region_id>(0), region);

	cmzn_scene_id scene = cmzn_region_get_scene(region);
	EXPECT_NE(static_cast<cmzn_scene_id>(0), scene);
	cmzn_selectionnotifier_id selectionnotifier = cmzn_scene_create_selectionnotifier(zinc.scene);
	EXPECT_NE(static_cast<cmzn_selectionnotifier_id>(0), selectionnotifier);
	cmzn_scene_destroy(&scene);

	RecordChange recordChange;
	recordChange.changeType = CMZN_SELECTIONEVENT_CHANGE_NONE;
	EXPECT_EQ(CMZN_OK, result = cmzn_selectionnotifier_set_callback(selectionnotifier,
		selectionCallback, static_cast<void*>(&recordChange)));

	cmzn_region_remove_child(zinc.root_region, region);
	cmzn_region_destroy(&region);

	EXPECT_EQ(CMZN_OK, result = cmzn_selectionnotifier_clear_callback(selectionnotifier));

	EXPECT_EQ(CMZN_OK, result = cmzn_selectionnotifier_destroy(&selectionnotifier));
}

class SelectioncallbackRecordChange : public Selectioncallback
{
	Selectionevent::ChangeType changeType;

	virtual void operator()(const Selectionevent &selectionevent)
	{
		this->changeType = selectionevent.getChangeType();
	}

public:
	SelectioncallbackRecordChange() :
			Selectioncallback(),
			changeType(Selectionevent::CHANGE_NONE)
	{ }

	void clearChange()
	{
		changeType = Selectionevent::CHANGE_NONE;
	}

	Selectionevent::ChangeType getChange() const
	{
		return this->changeType;
	}

};

TEST(ZincSelectionnotifier, changeCallback)
{
	ZincTestSetupCpp zinc;
	int result;

	Selectionnotifier selectionnotifier = zinc.scene.createSelectionnotifier();
	EXPECT_TRUE(selectionnotifier.isValid());

	EXPECT_FALSE(selectionnotifier.isHierarchical());
	EXPECT_EQ(OK, result = selectionnotifier.setHierarchical(true));
	EXPECT_TRUE(selectionnotifier.isHierarchical());

	SelectioncallbackRecordChange callback;
	EXPECT_EQ(OK, result = selectionnotifier.setCallback(callback));

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());
	EXPECT_EQ(OK, result = zinc.scene.setSelectionGroup(group));
	FieldGroup temp = zinc.scene.getSelectionGroup();
	EXPECT_EQ(group, temp);
	EXPECT_EQ(Selectionevent::CHANGE_NONE, callback.getChange());
	EXPECT_FALSE(group.containsLocalRegion());

	EXPECT_EQ(CMZN_OK, result = group.addLocalRegion());
	EXPECT_EQ(Selectionevent::CHANGE_ADD, callback.getChange());
	EXPECT_TRUE(group.containsLocalRegion());

	EXPECT_EQ(CMZN_OK, result = group.removeLocalRegion());
	EXPECT_EQ(Selectionevent::CHANGE_CLEAR, callback.getChange());
	EXPECT_FALSE(group.containsLocalRegion());

	EXPECT_EQ(OK, result = selectionnotifier.clearCallback());
}
