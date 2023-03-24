/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <opencmiss/zinc/field.h>
#include <opencmiss/zinc/fieldcache.h>
#include <opencmiss/zinc/fieldconstant.h>
#include <opencmiss/zinc/fieldfiniteelement.h>
#include <opencmiss/zinc/fieldvectoroperators.h>
#include <opencmiss/zinc/status.h>

#include <opencmiss/zinc/field.hpp>
#include <opencmiss/zinc/fieldarithmeticoperators.hpp>
#include <opencmiss/zinc/fieldcache.hpp>
#include <opencmiss/zinc/fieldconstant.hpp>
#include <opencmiss/zinc/fieldvectoroperators.hpp>
#include <opencmiss/zinc/status.hpp>

#include "test_resources.h"
#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

struct RecordChange
{
	cmzn_fieldmoduleevent_id lastEvent;
	int eventCount;

	RecordChange() :
		lastEvent(nullptr),
		eventCount(0)
	{
	}

	~RecordChange()
	{
		this->clear();
	}

	void clear()
	{
		cmzn_fieldmoduleevent_destroy(&this->lastEvent);
		eventCount = 0;
	}
};

void fieldmoduleCallback(cmzn_fieldmoduleevent_id event,
	void *recordChangeVoid)
{
	RecordChange *recordChange = reinterpret_cast<RecordChange*>(recordChangeVoid);
	if (recordChange->lastEvent)
	{
		cmzn_fieldmoduleevent_destroy(&recordChange->lastEvent);
	}
	recordChange->lastEvent = cmzn_fieldmoduleevent_access(event);
	++(recordChange->eventCount);
}

TEST(cmzn_fieldmodulenotifier, change_callback)
{
	ZincTestSetup zinc;
	int result;

	cmzn_fieldmodulenotifier_id notifier = cmzn_fieldmodule_create_fieldmodulenotifier(zinc.fm);
	EXPECT_NE(static_cast<cmzn_fieldmodulenotifier_id>(0), notifier);

	RecordChange recordChange;
	EXPECT_EQ(CMZN_OK, result = cmzn_fieldmodulenotifier_set_callback(notifier,
		fieldmoduleCallback, static_cast<void*>(&recordChange)));

	void *tmp = cmzn_fieldmodulenotifier_get_callback_user_data(notifier);
	EXPECT_EQ(static_cast<void*>(&recordChange), tmp);

	const double value1 = 2.0;
	cmzn_field_id joe = cmzn_fieldmodule_create_field_constant(zinc.fm, 1, &value1);
	EXPECT_NE((cmzn_field_id)0, joe);
	EXPECT_EQ(CMZN_FIELD_CHANGE_FLAG_ADD, result = cmzn_fieldmoduleevent_get_summary_field_change_flags(recordChange.lastEvent));

	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_name(joe, "joe"));
	EXPECT_EQ(CMZN_FIELD_CHANGE_FLAG_IDENTIFIER, result = cmzn_fieldmoduleevent_get_summary_field_change_flags(recordChange.lastEvent));

	cmzn_fieldcache_id cache = cmzn_fieldmodule_create_fieldcache(zinc.fm);
	double value2 = 4.5;
	EXPECT_EQ(CMZN_OK, result = cmzn_field_assign_real(joe, cache, 1, &value2));
	EXPECT_EQ(CMZN_FIELD_CHANGE_FLAG_DEFINITION | CMZN_FIELD_CHANGE_FLAG_FULL_RESULT, result = cmzn_fieldmoduleevent_get_summary_field_change_flags(recordChange.lastEvent));

	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_managed(joe, true));
	EXPECT_EQ(CMZN_FIELD_CHANGE_FLAG_DEFINITION, result = cmzn_fieldmoduleevent_get_summary_field_change_flags(recordChange.lastEvent));

	cmzn_field_id fred = cmzn_fieldmodule_create_field_magnitude(zinc.fm, joe);
	EXPECT_NE((cmzn_field_id)0, fred);
	EXPECT_EQ(CMZN_FIELD_CHANGE_FLAG_ADD, result = cmzn_fieldmoduleevent_get_summary_field_change_flags(recordChange.lastEvent));
	EXPECT_EQ(Field::CHANGE_FLAG_ADD, result = cmzn_fieldmoduleevent_get_field_change_flags(recordChange.lastEvent, fred));
	EXPECT_EQ(Field::CHANGE_FLAG_NONE, result = cmzn_fieldmoduleevent_get_field_change_flags(recordChange.lastEvent, joe));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_name(fred, "fred"));

	EXPECT_EQ(CMZN_OK, result = cmzn_field_assign_real(joe, cache, 1, &value1));
	EXPECT_EQ(CMZN_FIELD_CHANGE_FLAG_DEFINITION | CMZN_FIELD_CHANGE_FLAG_FULL_RESULT,
		result = cmzn_fieldmoduleevent_get_summary_field_change_flags(recordChange.lastEvent));
	EXPECT_EQ(CMZN_FIELD_CHANGE_FLAG_DEFINITION | CMZN_FIELD_CHANGE_FLAG_FULL_RESULT,
		result = cmzn_fieldmoduleevent_get_field_change_flags(recordChange.lastEvent, joe));
	EXPECT_EQ(CMZN_FIELD_CHANGE_FLAG_FULL_RESULT, result = cmzn_fieldmoduleevent_get_field_change_flags(recordChange.lastEvent, fred));

	EXPECT_EQ(CMZN_OK, cmzn_field_set_managed(joe, false));
	// keeping Fieldmoduleevent around is bad behaviour. Calling clear will give a single remove event
	// which is tested elsewhere, but this is a more severe test where the first notification clears
	// the last Fieldmoduleevent which releases field joe to send the second notification.
	//recordChange.clear();
	recordChange.eventCount = 0;
	cmzn_field_destroy(&joe);
	cmzn_field_destroy(&fred);
	EXPECT_EQ(CMZN_FIELD_CHANGE_FLAG_REMOVE, result = cmzn_fieldmoduleevent_get_summary_field_change_flags(recordChange.lastEvent));
	EXPECT_EQ(2, recordChange.eventCount);

	joe = cmzn_fieldmodule_find_field_by_name(zinc.fm, "joe");
	EXPECT_EQ(nullptr, joe);
	fred = cmzn_fieldmodule_find_field_by_name(zinc.fm, "fred");
	EXPECT_EQ(nullptr, fred);

	cmzn_fieldcache_destroy(&cache);
	EXPECT_EQ(CMZN_OK, result = cmzn_fieldmodulenotifier_clear_callback(notifier));
	EXPECT_EQ(CMZN_OK, result = cmzn_fieldmodulenotifier_destroy(&notifier));
}

// check selection callbacks safely handle destruction of scene
TEST(cmzn_fieldmodulenotifier, destroy_region_final_callback)
{
	ZincTestSetup zinc;
	int result;

	cmzn_region_id region = cmzn_region_create_child(zinc.root_region, "child");
	EXPECT_NE(static_cast<cmzn_region_id>(0), region);
	cmzn_fieldmodule_id fm = cmzn_region_get_fieldmodule(region);

	cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(fm, CMZN_FIELD_DOMAIN_TYPE_NODES);
	EXPECT_NE(static_cast<cmzn_nodeset_id>(0), nodeset);
	cmzn_mesh_id mesh = cmzn_fieldmodule_find_mesh_by_dimension(fm, 3);
	EXPECT_NE(static_cast<cmzn_mesh_id>(0), mesh);

	cmzn_fieldmodulenotifier_id notifier = cmzn_fieldmodule_create_fieldmodulenotifier(fm);
	EXPECT_NE(static_cast<cmzn_fieldmodulenotifier_id>(0), notifier);

	RecordChange recordChange;
	EXPECT_EQ(CMZN_OK, result = cmzn_fieldmodulenotifier_set_callback(notifier,
		fieldmoduleCallback, static_cast<void*>(&recordChange)));

	cmzn_region_remove_child(zinc.root_region, region);
	EXPECT_EQ(CMZN_FIELD_CHANGE_FLAG_NONE, result = cmzn_fieldmoduleevent_get_summary_field_change_flags(recordChange.lastEvent));

	cmzn_fieldmodule_destroy(&fm);
	cmzn_region_destroy(&region);
	EXPECT_EQ(CMZN_FIELD_CHANGE_FLAG_FINAL, result = cmzn_fieldmoduleevent_get_summary_field_change_flags(recordChange.lastEvent));

	// check no nodeset or mesh changes with final notification:
	cmzn_nodesetchanges_id nodesetchanges = cmzn_fieldmoduleevent_get_nodesetchanges(recordChange.lastEvent, nodeset);
	EXPECT_EQ(static_cast<cmzn_nodesetchanges_id>(0), nodesetchanges);
	cmzn_meshchanges_id meshchanges = cmzn_fieldmoduleevent_get_meshchanges(recordChange.lastEvent, mesh);
	EXPECT_EQ(static_cast<cmzn_meshchanges_id>(0), meshchanges);

	EXPECT_EQ(CMZN_OK, result = cmzn_fieldmodulenotifier_destroy(&notifier));
	cmzn_nodeset_destroy(&nodeset);
	cmzn_mesh_destroy(&mesh);
}

class FieldmodulecallbackRecordChange : public Fieldmodulecallback
{
public:
	Fieldmoduleevent lastEvent;
	int eventCount;

	FieldmodulecallbackRecordChange() :
		eventCount(0)
	{ }

	virtual void operator()(const Fieldmoduleevent &event)
	{
		this->lastEvent = event;
		++eventCount;
	}

	void clear()
	{
		lastEvent = Fieldmoduleevent();
		eventCount = 0;
	}
};

TEST(ZincFieldmodulenotifier, changeCallback)
{
	ZincTestSetupCpp zinc;
	int result;

	Fieldmodulenotifier notifier = zinc.fm.createFieldmodulenotifier();
	EXPECT_TRUE(notifier.isValid());

	FieldmodulecallbackRecordChange recordChange;
	EXPECT_EQ(CMZN_OK, result = notifier.setCallback(recordChange));

	const double value1 = 2.0;
	Field joe = zinc.fm.createFieldConstant(1, &value1);
	EXPECT_TRUE(joe.isValid());
	EXPECT_EQ(Field::CHANGE_FLAG_ADD, result = recordChange.lastEvent.getSummaryFieldChangeFlags());

	EXPECT_EQ(CMZN_OK, result = joe.setName("joe"));
	EXPECT_EQ(Field::CHANGE_FLAG_IDENTIFIER, result = recordChange.lastEvent.getSummaryFieldChangeFlags());

	Fieldcache cache = zinc.fm.createFieldcache();
	double value2 = 4.5;
	EXPECT_EQ(CMZN_OK, result = joe.assignReal(cache, 1, &value2));
	EXPECT_EQ(Field::CHANGE_FLAG_DEFINITION | Field::CHANGE_FLAG_FULL_RESULT,
		result = recordChange.lastEvent.getSummaryFieldChangeFlags());

	EXPECT_EQ(CMZN_OK, result = joe.setManaged(true));
	EXPECT_EQ(Field::CHANGE_FLAG_DEFINITION, result = recordChange.lastEvent.getSummaryFieldChangeFlags());

	Field fred = zinc.fm.createFieldMagnitude(joe);
	EXPECT_TRUE(fred.isValid());
	EXPECT_EQ(Field::CHANGE_FLAG_ADD, result = recordChange.lastEvent.getSummaryFieldChangeFlags());
	EXPECT_EQ(Field::CHANGE_FLAG_ADD, result = recordChange.lastEvent.getFieldChangeFlags(fred));
	EXPECT_EQ(Field::CHANGE_FLAG_NONE, result = recordChange.lastEvent.getFieldChangeFlags(joe));

	EXPECT_EQ(CMZN_OK, result = joe.assignReal(cache, 1, &value1));
	EXPECT_EQ(Field::CHANGE_FLAG_DEFINITION | Field::CHANGE_FLAG_FULL_RESULT,
		result = recordChange.lastEvent.getSummaryFieldChangeFlags());
	EXPECT_EQ(Field::CHANGE_FLAG_DEFINITION | Field::CHANGE_FLAG_FULL_RESULT,
		result = recordChange.lastEvent.getFieldChangeFlags(joe));
	EXPECT_EQ(Field::CHANGE_FLAG_FULL_RESULT, result = recordChange.lastEvent.getFieldChangeFlags(fred));

	EXPECT_EQ(CMZN_OK, result = joe.setManaged(false));
	// keeping Fieldmoduleevent around is bad behaviour. Calling clear will give a single remove event
	// which is tested elsewhere, but this is a more severe test where the first notification clears
	// the last Fieldmoduleevent which releases field joe to send the second notification.
	//recordChange.clear();
	recordChange.eventCount = 0;
	joe = Field();
	fred = Field();
	EXPECT_EQ(Field::CHANGE_FLAG_REMOVE, result = recordChange.lastEvent.getSummaryFieldChangeFlags());
	EXPECT_EQ(2, recordChange.eventCount);
	joe = zinc.fm.findFieldByName("joe");
	EXPECT_FALSE(joe.isValid());
	fred = zinc.fm.findFieldByName("fred");
	EXPECT_FALSE(joe.isValid());

	EXPECT_EQ(CMZN_OK, result = notifier.clearCallback());
}

void createNodesWithCoordinates(cmzn_fieldmodule_id fm)
{
	int result;
	cmzn_fieldmodule_begin_change(fm);
	cmzn_field_id field = cmzn_fieldmodule_create_field_finite_element(fm, /*number_of_components*/3);
	EXPECT_NE(static_cast<cmzn_field_id>(0), field);
	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_name(field, "coordinates"));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_type_coordinate(field, true));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_managed(field, true));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_component_name(field, 1, "x"));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_component_name(field, 2, "y"));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_component_name(field, 3, "z"));

	cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(fm, CMZN_FIELD_DOMAIN_TYPE_NODES);
	EXPECT_NE(static_cast<cmzn_nodeset_id>(0), nodeset);
	cmzn_nodetemplate_id nodetemplate = cmzn_nodeset_create_nodetemplate(nodeset);
	EXPECT_NE(static_cast<cmzn_nodetemplate_id>(0), nodetemplate);
	EXPECT_EQ(CMZN_OK, result = cmzn_nodetemplate_define_field(nodetemplate, field));
	double nodeCoordinates[4][3] =
	{
		{ 0.0, 0.0, 0.0 },
		{ 1.0, 0.0, 0.0 },
		{ 0.0, 1.0, 0.0 },
		{ 1.0, 1.0, 0.0 }
	};
	cmzn_fieldcache_id cache = cmzn_fieldmodule_create_fieldcache(fm);
	for (int i = 1; i <= 4; ++i)
	{
		cmzn_node_id node = cmzn_nodeset_create_node(nodeset, i, nodetemplate);
		EXPECT_NE(static_cast<cmzn_node_id>(0), node);
		EXPECT_EQ(CMZN_OK, result = cmzn_fieldcache_set_node(cache, node));
		EXPECT_EQ(CMZN_OK, result = cmzn_field_assign_real(field, cache, 3, nodeCoordinates[i - 1]));
		EXPECT_EQ(CMZN_OK, result = cmzn_node_destroy(&node));
	}
	cmzn_fieldcache_destroy(&cache);
	cmzn_nodetemplate_destroy(&nodetemplate);
	cmzn_nodeset_destroy(&nodeset);

	cmzn_field_destroy(&field);
	cmzn_fieldmodule_end_change(fm);
}

TEST(ZincFieldmodulenotifier, partial_nodeset_change)
{
	ZincTestSetupCpp zinc;
	int result;

	Fieldmodulenotifier notifier = zinc.fm.createFieldmodulenotifier();
	EXPECT_TRUE(notifier.isValid());
	FieldmodulecallbackRecordChange recordChange;
	EXPECT_EQ(CMZN_OK, result = notifier.setCallback(recordChange));

	createNodesWithCoordinates(zinc.fm.getId());
	EXPECT_EQ(Field::CHANGE_FLAG_ADD | Field::CHANGE_FLAG_PARTIAL_RESULT, result = recordChange.lastEvent.getSummaryFieldChangeFlags());
	Nodeset nodeset = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	Node node1 = nodeset.findNodeByIdentifier(1);
	EXPECT_TRUE(node1.isValid());
	Node node2 = nodeset.findNodeByIdentifier(2);
	EXPECT_TRUE(node2.isValid());
	Nodesetchanges nodesetchanges = recordChange.lastEvent.getNodesetchanges(nodeset);
	EXPECT_TRUE(nodesetchanges.isValid());
	// following could change with internal logic:
	EXPECT_EQ(4, result = nodesetchanges.getNumberOfChanges());
	EXPECT_EQ(Node::CHANGE_FLAG_ADD | Node::CHANGE_FLAG_FIELD, result = nodesetchanges.getSummaryNodeChangeFlags());
	EXPECT_EQ(Node::CHANGE_FLAG_ADD | Node::CHANGE_FLAG_FIELD, result = nodesetchanges.getNodeChangeFlags(node2));
	EXPECT_EQ(Node::CHANGE_FLAG_ADD | Node::CHANGE_FLAG_FIELD, result = nodesetchanges.getNodeChangeFlags(node1));

	Field field = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(field.isValid());

	double newCoordinates[3] = { 1.5, 0.2, 0.3 };
	Fieldcache cache = zinc.fm.createFieldcache();
	EXPECT_EQ(CMZN_OK, result = cache.setNode(node2));
	EXPECT_EQ(CMZN_OK, result = field.assignReal(cache, 3, newCoordinates));
	EXPECT_EQ(Field::CHANGE_FLAG_PARTIAL_RESULT, result = recordChange.lastEvent.getSummaryFieldChangeFlags());

	nodesetchanges = recordChange.lastEvent.getNodesetchanges(nodeset);
	EXPECT_TRUE(nodesetchanges.isValid());
	EXPECT_EQ(1, result = nodesetchanges.getNumberOfChanges());
 	EXPECT_EQ(Node::CHANGE_FLAG_FIELD, result = nodesetchanges.getSummaryNodeChangeFlags());
	EXPECT_EQ(Node::CHANGE_FLAG_FIELD, result = nodesetchanges.getNodeChangeFlags(node2));
	EXPECT_EQ(Node::CHANGE_FLAG_NONE, result = nodesetchanges.getNodeChangeFlags(node1));

	EXPECT_EQ(CMZN_OK, result = notifier.clearCallback());
}

TEST(ZincFieldmodulenotifier, partial_mesh_change)
{
	ZincTestSetupCpp zinc;
	int result;

	Fieldmodulenotifier notifier = zinc.fm.createFieldmodulenotifier();
	EXPECT_TRUE(notifier.isValid());
	FieldmodulecallbackRecordChange recordChange;
	EXPECT_EQ(CMZN_OK, result = notifier.setCallback(recordChange));

    EXPECT_EQ(CMZN_OK, result = zinc.root_region.readFile(resourcePath("fieldmodule/two_cubes.exformat").c_str()));
	EXPECT_EQ(Field::CHANGE_FLAG_ADD | Field::CHANGE_FLAG_PARTIAL_RESULT, result = recordChange.lastEvent.getSummaryFieldChangeFlags());

	Nodeset nodeset = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	Node node1 = nodeset.findNodeByIdentifier(1);
	EXPECT_TRUE(node1.isValid());
	Node node2 = nodeset.findNodeByIdentifier(2);
	EXPECT_TRUE(node2.isValid());

	Field field = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(field.isValid());

	double newCoordinates[3] = { 1.0, 0.3, 1.5 };
	Fieldcache cache = zinc.fm.createFieldcache();
	EXPECT_EQ(CMZN_OK, result = cache.setNode(node1));
	EXPECT_EQ(CMZN_OK, result = field.assignReal(cache, 3, newCoordinates));
	EXPECT_EQ(Field::CHANGE_FLAG_PARTIAL_RESULT, result = recordChange.lastEvent.getSummaryFieldChangeFlags());
	EXPECT_EQ(Field::CHANGE_FLAG_PARTIAL_RESULT, result = recordChange.lastEvent.getFieldChangeFlags(field));

	Nodesetchanges nodesetchanges = recordChange.lastEvent.getNodesetchanges(nodeset);
	EXPECT_TRUE(nodesetchanges.isValid());
	EXPECT_EQ(1, result = nodesetchanges.getNumberOfChanges());
 	EXPECT_EQ(Node::CHANGE_FLAG_FIELD, result = nodesetchanges.getSummaryNodeChangeFlags());
	EXPECT_EQ(Node::CHANGE_FLAG_FIELD, result = nodesetchanges.getNodeChangeFlags(node1));
	EXPECT_EQ(Node::CHANGE_FLAG_NONE, result = nodesetchanges.getNodeChangeFlags(node2));

	Mesh mesh3 = zinc.fm.findMeshByDimension(3);
	EXPECT_TRUE(mesh3.isValid());
	Meshchanges mesh3changes = recordChange.lastEvent.getMeshchanges(mesh3);
	EXPECT_TRUE(mesh3changes.isValid());
	EXPECT_EQ(Element::CHANGE_FLAG_FIELD, result = mesh3changes.getSummaryElementChangeFlags());
	Element::ChangeFlags elementResults[2] =
	{
		Element::CHANGE_FLAG_FIELD,
		Element::CHANGE_FLAG_NONE
	};
	EXPECT_EQ(2, mesh3.getSize());
	for (int el = 1; el <= 2; ++el)
	{
		Element element = mesh3.findElementByIdentifier(el);
		EXPECT_TRUE(element.isValid());
		EXPECT_EQ(elementResults[el - 1], mesh3changes.getElementChangeFlags(element));
	}

	Mesh mesh2 = zinc.fm.findMeshByDimension(2);
	EXPECT_TRUE(mesh2.isValid());
	Meshchanges mesh2changes = recordChange.lastEvent.getMeshchanges(mesh2);
	EXPECT_TRUE(mesh2changes.isValid());
	EXPECT_EQ(Element::CHANGE_FLAG_FIELD, result = mesh2changes.getSummaryElementChangeFlags());
	Element::ChangeFlags faceResults[11] =
	{
		Element::CHANGE_FLAG_FIELD,
		Element::CHANGE_FLAG_FIELD,
		Element::CHANGE_FLAG_NONE,
		Element::CHANGE_FLAG_FIELD,
		Element::CHANGE_FLAG_FIELD,
		Element::CHANGE_FLAG_NONE,
		Element::CHANGE_FLAG_NONE,
		Element::CHANGE_FLAG_FIELD,
		Element::CHANGE_FLAG_FIELD,
		Element::CHANGE_FLAG_NONE,
		Element::CHANGE_FLAG_NONE
	};
	EXPECT_EQ(11, mesh2.getSize());
	for (int fa = 1; fa <= 11; ++fa)
	{
		Element face = mesh2.findElementByIdentifier(fa);
		EXPECT_TRUE(face.isValid());
		EXPECT_EQ(faceResults[fa - 1], mesh2changes.getElementChangeFlags(face));
	}

	Mesh mesh1 = zinc.fm.findMeshByDimension(1);
	EXPECT_TRUE(mesh1.isValid());
	Meshchanges mesh1changes = recordChange.lastEvent.getMeshchanges(mesh1);
	EXPECT_TRUE(mesh1changes.isValid());
	EXPECT_EQ(Element::CHANGE_FLAG_FIELD, result = mesh1changes.getSummaryElementChangeFlags());
	Element::ChangeFlags lineResults[20] =
	{
		Element::CHANGE_FLAG_FIELD,
		Element::CHANGE_FLAG_FIELD,
		Element::CHANGE_FLAG_FIELD,
		Element::CHANGE_FLAG_FIELD,
		Element::CHANGE_FLAG_NONE,
		Element::CHANGE_FLAG_NONE,
		Element::CHANGE_FLAG_FIELD,
		Element::CHANGE_FLAG_FIELD,
		Element::CHANGE_FLAG_FIELD,
		Element::CHANGE_FLAG_FIELD,
		Element::CHANGE_FLAG_NONE,
		Element::CHANGE_FLAG_NONE,
		Element::CHANGE_FLAG_FIELD,
		Element::CHANGE_FLAG_FIELD,
		Element::CHANGE_FLAG_FIELD,
		Element::CHANGE_FLAG_FIELD,
		Element::CHANGE_FLAG_NONE,
		Element::CHANGE_FLAG_NONE,
		Element::CHANGE_FLAG_NONE,
		Element::CHANGE_FLAG_NONE
	};
	EXPECT_EQ(20, mesh1.getSize());
	for (int li = 1; li <= 20; ++li)
	{
		Element line = mesh1.findElementByIdentifier(li);
		EXPECT_TRUE(line.isValid());
		EXPECT_EQ(lineResults[li - 1], mesh1changes.getElementChangeFlags(line));
	}

	EXPECT_EQ(CMZN_OK, result = notifier.clearCallback());
}


/* test defining faces correctly notifies for new faces, and changes to parents */
TEST(ZincFieldmodulenotifier, defineFaces)
{
	ZincTestSetupCpp zinc;
	int result;

    EXPECT_EQ(CMZN_OK, result = zinc.root_region.readFile(resourcePath("fieldmodule/part_surfaces.ex2").c_str()));
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_EQ(6, mesh3d.getSize());
	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	const int mesh2dSizeBefore = mesh2d.getSize();
	EXPECT_EQ(27, mesh2dSizeBefore);
	Mesh mesh1d = zinc.fm.findMeshByDimension(1);
	const int mesh1dSizeBefore = mesh1d.getSize();
	EXPECT_EQ(48, mesh1dSizeBefore);

	Fieldmodulenotifier notifier = zinc.fm.createFieldmodulenotifier();
	EXPECT_TRUE(notifier.isValid());
	FieldmodulecallbackRecordChange recordChange;
	EXPECT_EQ(CMZN_OK, result = notifier.setCallback(recordChange));

	// element 267 has no faces; define these
	EXPECT_EQ(RESULT_OK, zinc.fm.defineAllFaces());
	// test correct face and line additions
	const int mesh2dSizeAfter = mesh2d.getSize();
	EXPECT_EQ(32, mesh2dSizeAfter);
	const int mesh1dSizeAfter = mesh1d.getSize();
	EXPECT_EQ(56, mesh1dSizeAfter);

	// test correct notifications
	Element element;
	EXPECT_EQ(Field::CHANGE_FLAG_PARTIAL_RESULT, result = recordChange.lastEvent.getSummaryFieldChangeFlags());

	Meshchanges meshchanges3d = recordChange.lastEvent.getMeshchanges(mesh3d);
	result = meshchanges3d.getSummaryElementChangeFlags();
	EXPECT_EQ(Element::CHANGE_FLAG_DEFINITION, result);
	Elementiterator iter = mesh3d.createElementiterator();
	EXPECT_TRUE(iter.isValid());
	while ((element = iter.next()).isValid())
	{
		result = meshchanges3d.getElementChangeFlags(element);
		if (element.getIdentifier() == 267)
		{
			EXPECT_EQ(Element::CHANGE_FLAG_DEFINITION, result);
		}
		else
		{
			EXPECT_EQ(Element::CHANGE_FLAG_NONE, result);
		}
	}

	Meshchanges meshchanges2d = recordChange.lastEvent.getMeshchanges(mesh2d);
	const int expectedresult2d = Element::CHANGE_FLAG_ADD | Element::CHANGE_FLAG_DEFINITION | Element::CHANGE_FLAG_FIELD;
	result = meshchanges2d.getSummaryElementChangeFlags();
	EXPECT_EQ(expectedresult2d, result);
	iter = mesh2d.createElementiterator();
	EXPECT_TRUE(iter.isValid());
	while ((element = iter.next()).isValid())
	{
		result = meshchanges2d.getElementChangeFlags(element);
		const int identifier = element.getIdentifier();
		// faces 1-5 are new; 1 existing face has parent change propagated to it
		if ((identifier <= 5) || (identifier == 1019))
		{
			EXPECT_EQ(expectedresult2d, result);
		}
		else
		{
			EXPECT_EQ(Element::CHANGE_FLAG_NONE, result);
		}
	}

	Meshchanges meshchanges1d = recordChange.lastEvent.getMeshchanges(mesh1d);
	result = meshchanges1d.getSummaryElementChangeFlags();
	const int expectedresult1d = Element::CHANGE_FLAG_ADD | Element::CHANGE_FLAG_FIELD;
	EXPECT_EQ(expectedresult1d, result);
	iter = mesh1d.createElementiterator();
	EXPECT_TRUE(iter.isValid());
	while ((element = iter.next()).isValid())
	{
		result = meshchanges1d.getElementChangeFlags(element);
		const int identifier = element.getIdentifier();
		// faces 1-8 are new; 4 existing lines have parent change propagated to them
		if ((identifier <= 8)
			|| (identifier == 1297)
			|| (identifier == 1302)
			|| (identifier == 1305)
			|| (identifier == 1306))
		{
			EXPECT_EQ(expectedresult1d, result);
		}
		else
		{
			EXPECT_EQ(Element::CHANGE_FLAG_NONE, result);
		}
	}
}

// Test destroying a field with source fields only referenced by it
// so a chain of fields should be removed with one notification.
TEST(ZincFieldmodulenotifier, destroyFieldChain)
{
	ZincTestSetupCpp zinc;
	int result;

	Fieldmodulenotifier notifier = zinc.fm.createFieldmodulenotifier();
	EXPECT_TRUE(notifier.isValid());

	FieldmodulecallbackRecordChange recordChange;
	EXPECT_EQ(RESULT_OK, result = notifier.setCallback(recordChange));

	recordChange.clear();
	zinc.fm.beginChange();
	const double value1 = 2.0;
	Field joe = zinc.fm.createFieldConstant(1, &value1);
	EXPECT_TRUE(joe.isValid());
	EXPECT_EQ(RESULT_OK, joe.setName("joe"));
	zinc.fm.endChange();
	EXPECT_EQ(Field::CHANGE_FLAG_ADD, result = recordChange.lastEvent.getSummaryFieldChangeFlags());
	EXPECT_EQ(1, recordChange.eventCount);
	recordChange.clear();

	recordChange.clear();
	zinc.fm.beginChange();
	const double value2 = 3.0;
	Field bob = zinc.fm.createFieldConstant(1, &value2);
	EXPECT_TRUE(bob.isValid());
	EXPECT_EQ(RESULT_OK, bob.setName("bob"));
	zinc.fm.endChange();
	EXPECT_EQ(Field::CHANGE_FLAG_ADD, result = recordChange.lastEvent.getSummaryFieldChangeFlags());
	EXPECT_EQ(1, recordChange.eventCount);
	recordChange.clear();

	recordChange.clear();
	zinc.fm.beginChange();
	Field alf = joe + bob;
	EXPECT_TRUE(alf.isValid());
	EXPECT_EQ(RESULT_OK, alf.setName("alf"));
	zinc.fm.endChange();
	EXPECT_EQ(Field::CHANGE_FLAG_ADD, result = recordChange.lastEvent.getSummaryFieldChangeFlags());
	EXPECT_EQ(1, recordChange.eventCount);
	recordChange.clear();

	recordChange.clear();
	zinc.fm.beginChange();
	Field fred = zinc.fm.createFieldSqrt(alf);
	EXPECT_TRUE(fred.isValid());
	EXPECT_EQ(RESULT_OK, fred.setName("fred"));
	zinc.fm.endChange();
	EXPECT_EQ(Field::CHANGE_FLAG_ADD, result = recordChange.lastEvent.getSummaryFieldChangeFlags());
	EXPECT_EQ(1, recordChange.eventCount);
	recordChange.clear();

	joe = Field();
	bob = Field();
	alf = Field();
	// this should destroy all fields in chain and produce 1 remove event:
	fred = Field();
	EXPECT_EQ(Field::CHANGE_FLAG_REMOVE, result = recordChange.lastEvent.getSummaryFieldChangeFlags());
	EXPECT_EQ(1, recordChange.eventCount);
	recordChange.clear();
}
