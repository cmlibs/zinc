
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/core.h>
#include <zinc/context.h>
#include <zinc/element.h>
#include <zinc/field.h>
#include <zinc/fieldmodule.h>
#include <zinc/fieldfiniteelement.h>
#include <zinc/node.h>
#include <zinc/region.h>
#include <zinc/status.h>
#include <zinc/stream.h>

#include <zinc/element.hpp>
#include <zinc/node.hpp>
#include <zinc/status.hpp>
#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

#include "test_resources.h"


TEST(nodes_elements_identifier, set_identifier)
{
	cmzn_context_id context = cmzn_context_create("test");
	cmzn_region_id root_region = cmzn_context_get_default_region(context);

	cmzn_region_id cube_region = cmzn_region_create_child(root_region, "cube");
	cmzn_stream_information_id cube_si = cmzn_region_create_stream_information(
		cube_region);
	EXPECT_NE(static_cast<cmzn_stream_information *>(0), cube_si);

	cmzn_stream_resource_id cube_sr = cmzn_stream_information_create_resource_file(
		cube_si, TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE));
	EXPECT_NE(static_cast<cmzn_stream_resource *>(0), cube_sr);

	int result = 0;
	EXPECT_EQ(CMZN_OK, result = cmzn_region_read(cube_region, cube_si));

	cmzn_stream_resource_destroy(&cube_sr);
	cmzn_stream_information_destroy(&cube_si);

	cmzn_fieldmodule_id cubeFM = cmzn_region_get_fieldmodule(cube_region);
	EXPECT_NE(static_cast<cmzn_fieldmodule *>(0), cubeFM);

	cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_name(cubeFM, "nodes");
	EXPECT_NE(static_cast<cmzn_nodeset *>(0), nodeset);

	cmzn_node_id node = cmzn_nodeset_find_node_by_identifier(nodeset, 1);
	EXPECT_NE(static_cast<cmzn_node *>(0), node);

	EXPECT_EQ(CMZN_ERROR_GENERAL, result = cmzn_node_set_identifier(node, 3));
	EXPECT_EQ(CMZN_OK, result = cmzn_node_set_identifier(node, 9));

	cmzn_mesh_id mesh = cmzn_fieldmodule_find_mesh_by_dimension(cubeFM, 3);
	EXPECT_NE(static_cast<cmzn_mesh *>(0), mesh);

	cmzn_element_id element = cmzn_mesh_find_element_by_identifier(mesh, 1);
	EXPECT_NE(static_cast<cmzn_element *>(0), element);

	EXPECT_EQ(CMZN_ERROR_GENERAL, result = cmzn_element_set_identifier(element, 1));
	EXPECT_EQ(CMZN_OK, result = cmzn_element_set_identifier(element, 2));

	cmzn_element_destroy(&element);
	cmzn_mesh_destroy(&mesh);
	cmzn_fieldmodule_destroy(&cubeFM);
	cmzn_nodeset_destroy(&nodeset);
	cmzn_node_destroy(&node);

	cmzn_region_destroy(&cube_region);
	cmzn_region_destroy(&root_region);
	cmzn_context_destroy(&context);
}

TEST(ZincElementiterator, iteration)
{
	ZincTestSetupCpp zinc;

	Mesh mesh = zinc.fm.findMeshByDimension(3);
	Elementtemplate elementTemplate = mesh.createElementtemplate();
	EXPECT_TRUE(elementTemplate.isValid());
	EXPECT_EQ(OK, elementTemplate.setShapeType(Element::SHAPE_CUBE));

	EXPECT_EQ(OK, mesh.defineElement(4, elementTemplate));
	Element e4 = mesh.findElementByIdentifier(4);
	EXPECT_TRUE(e4.isValid());
	EXPECT_EQ(OK, mesh.defineElement(1, elementTemplate));
	Element e1 = mesh.findElementByIdentifier(1);
	EXPECT_TRUE(e1.isValid());
	EXPECT_EQ(OK, mesh.defineElement(200, elementTemplate));
	Element e200 = mesh.findElementByIdentifier(200);
	EXPECT_TRUE(e200.isValid());

	Elementiterator iter = mesh.createElementiterator();
	EXPECT_TRUE(iter.isValid());
	Element e;
	e = iter.next();
	EXPECT_EQ(e1, e);
	EXPECT_EQ(1, e.getIdentifier());
	e = iter.next();
	EXPECT_EQ(e4, e);
	EXPECT_EQ(4, e.getIdentifier());
	e = iter.next();
	EXPECT_EQ(e200, e);
	EXPECT_EQ(200, e.getIdentifier());
}

TEST(ZincNodeiterator, iteration)
{
	ZincTestSetupCpp zinc;

	Nodeset nodeset = zinc.fm.findNodesetByDomainType(Field::DOMAIN_NODES);
	Nodetemplate nodeTemplate = nodeset.createNodetemplate();
	EXPECT_TRUE(nodeTemplate.isValid());

	Node n4 = nodeset.createNode(4, nodeTemplate);
	EXPECT_TRUE(n4.isValid());
	Node n1 = nodeset.createNode(1, nodeTemplate);
	EXPECT_TRUE(n1.isValid());
	Node n200 = nodeset.createNode(200, nodeTemplate);
	EXPECT_TRUE(n200.isValid());

	Nodeiterator iter = nodeset.createNodeiterator();
	EXPECT_TRUE(iter.isValid());
	Node n;
	n = iter.next();
	EXPECT_EQ(n1, n);
	EXPECT_EQ(1, n.getIdentifier());
	n = iter.next();
	EXPECT_EQ(n4, n);
	EXPECT_EQ(4, n.getIdentifier());
	n = iter.next();
	EXPECT_EQ(n200, n);
	EXPECT_EQ(200, n.getIdentifier());
}
