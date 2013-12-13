#include <gtest/gtest.h>

#include <zinc/field.h>
#include <zinc/fieldcache.h>
#include <zinc/fieldconstant.h>
#include <zinc/fieldfiniteelement.h>
#include <zinc/fieldvectoroperators.h>
#include <zinc/status.h>

#include <zinc/field.hpp>
#include <zinc/fieldcache.hpp>
#include <zinc/fieldconstant.hpp>
#include <zinc/fieldvectoroperators.hpp>
#include <zinc/status.hpp>

#include "test_resources.h"
#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

struct RecordChange
{
	cmzn_fieldmoduleevent_id lastEvent;

	RecordChange() :
		lastEvent(0)
	{
	}

	~RecordChange()
	{
		cmzn_fieldmoduleevent_destroy(&lastEvent);
	}
};

void fieldmoduleCallback(cmzn_fieldmoduleevent_id event,
	void *recordChangeVoid)
{
	RecordChange *recordChange = reinterpret_cast<RecordChange*>(recordChangeVoid);
	if (recordChange->lastEvent)
		cmzn_fieldmoduleevent_destroy(&recordChange->lastEvent);
	recordChange->lastEvent = cmzn_fieldmoduleevent_access(event);
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

	EXPECT_EQ(CMZN_OK, result = cmzn_field_assign_real(joe, cache, 1, &value1));
	EXPECT_EQ(CMZN_FIELD_CHANGE_FLAG_DEFINITION | CMZN_FIELD_CHANGE_FLAG_FULL_RESULT,
		result = cmzn_fieldmoduleevent_get_summary_field_change_flags(recordChange.lastEvent));
	EXPECT_EQ(CMZN_FIELD_CHANGE_FLAG_DEFINITION | CMZN_FIELD_CHANGE_FLAG_FULL_RESULT,
		result = cmzn_fieldmoduleevent_get_field_change_flags(recordChange.lastEvent, joe));
	EXPECT_EQ(CMZN_FIELD_CHANGE_FLAG_FULL_RESULT, result = cmzn_fieldmoduleevent_get_field_change_flags(recordChange.lastEvent, fred));

	EXPECT_EQ(CMZN_OK, cmzn_field_set_managed(joe, false));
	cmzn_field_destroy(&joe);
	cmzn_field_destroy(&fred);
	EXPECT_EQ(CMZN_FIELD_CHANGE_FLAG_REMOVE, result = cmzn_fieldmoduleevent_get_summary_field_change_flags(recordChange.lastEvent));

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

	EXPECT_EQ(CMZN_OK, result = cmzn_fieldmodulenotifier_destroy(&notifier));
}

class FieldmodulecallbackRecordChange : public Fieldmodulecallback
{
public:
	Fieldmoduleevent lastEvent;

	FieldmodulecallbackRecordChange()
	{ }

	virtual void operator()(const Fieldmoduleevent &event)
	{
		this->lastEvent = event;
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
	Field noField;
	joe = noField;
	fred = noField;
	EXPECT_EQ(Field::CHANGE_FLAG_REMOVE, result = recordChange.lastEvent.getSummaryFieldChangeFlags());

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
	EXPECT_EQ(16, result = nodesetchanges.getNumberOfChanges());
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

	EXPECT_EQ(CMZN_OK, result = zinc.root_region.readFile(TestResources::getLocation(TestResources::FIELDMODULE_TWO_CUBES_RESOURCE)));
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
