/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

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
#include <zinc/streamregion.h>

#include <zinc/context.hpp>
#include <zinc/element.hpp>
#include <zinc/node.hpp>
#include <zinc/status.hpp>
#include <zinc/stream.hpp>
#include <zinc/streamregion.hpp>
#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

#include "test_resources.h"


TEST(nodes_elements_identifier, set_identifier)
{
	ZincTestSetup zinc;

	cmzn_region_id cube_region = cmzn_region_create_child(zinc.root_region, "cube");
	cmzn_streaminformation_id cube_si = cmzn_region_create_streaminformation(
		cube_region);
	EXPECT_NE(static_cast<cmzn_streaminformation *>(0), cube_si);

	cmzn_streamresource_id cube_sr = cmzn_streaminformation_create_streamresource_file(
		cube_si, TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE));
	EXPECT_NE(static_cast<cmzn_streamresource *>(0), cube_sr);

	cmzn_streaminformation_region_id cube_si_region = cmzn_streaminformation_cast_region(
		cube_si);
	EXPECT_NE(static_cast<cmzn_streaminformation_region *>(0), cube_si_region);

	int result = 0;
	EXPECT_EQ(CMZN_OK, result = cmzn_region_read(cube_region, cube_si_region));

	cmzn_streamresource_destroy(&cube_sr);
	cmzn_streaminformation_region_destroy(&cube_si_region);
	cmzn_streaminformation_destroy(&cube_si);

	cmzn_fieldmodule_id cubeFm = cmzn_region_get_fieldmodule(cube_region);
	EXPECT_NE(static_cast<cmzn_fieldmodule *>(0), cubeFm);

	cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(cubeFm, CMZN_FIELD_DOMAIN_TYPE_NODES);
	EXPECT_NE(static_cast<cmzn_nodeset *>(0), nodeset);

	cmzn_node_id node = cmzn_nodeset_find_node_by_identifier(nodeset, 1);
	EXPECT_NE(static_cast<cmzn_node *>(0), node);

	EXPECT_EQ(CMZN_ERROR_GENERAL, result = cmzn_node_set_identifier(node, 3));
	EXPECT_EQ(CMZN_OK, result = cmzn_node_set_identifier(node, 9));

	cmzn_mesh_id mesh = cmzn_fieldmodule_find_mesh_by_dimension(cubeFm, 3);
	EXPECT_NE(static_cast<cmzn_mesh *>(0), mesh);

	cmzn_element_id element = cmzn_mesh_find_element_by_identifier(mesh, 1);
	EXPECT_NE(static_cast<cmzn_element *>(0), element);

	EXPECT_EQ(CMZN_ERROR_GENERAL, result = cmzn_element_set_identifier(element, 1));
	EXPECT_EQ(CMZN_OK, result = cmzn_element_set_identifier(element, 2));

	cmzn_element_destroy(&element);
	cmzn_mesh_destroy(&mesh);
	cmzn_fieldmodule_destroy(&cubeFm);
	cmzn_nodeset_destroy(&nodeset);
	cmzn_node_destroy(&node);

	cmzn_region_destroy(&cube_region);
}

TEST(ZincNodesElements, setIdentifier)
{
	ZincTestSetupCpp zinc;

	Region cubeRegion = zinc.root_region.createChild("cube");
	StreaminformationRegion cubeSi = cubeRegion.createStreaminformation();
	EXPECT_TRUE(cubeSi.isValid());

	Streamresource cubeSr = cubeSi.createStreamresourceFile(
		TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE));
	EXPECT_TRUE(cubeSr.isValid());

	int result;
	EXPECT_EQ(OK, result = cubeRegion.read(cubeSi));

	Fieldmodule cubeFm = cubeRegion.getFieldmodule();
	EXPECT_TRUE(cubeFm.isValid());

	Nodeset nodeset = cubeFm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodeset.isValid());

	Node node = nodeset.findNodeByIdentifier(1);
	EXPECT_TRUE(node.isValid());

	EXPECT_EQ(ERROR_GENERAL, result = node.setIdentifier(3));
	EXPECT_EQ(OK, result = node.setIdentifier(9));

	Mesh mesh = cubeFm.findMeshByDimension(3);
	EXPECT_TRUE(mesh.isValid());

	Element element = mesh.findElementByIdentifier(1);
	EXPECT_TRUE(element.isValid());

	EXPECT_EQ(ERROR_GENERAL, result = element.setIdentifier(1));
	EXPECT_EQ(OK, result = element.setIdentifier(2));
}

TEST(ZincElementiterator, iteration)
{
	ZincTestSetupCpp zinc;

	Mesh mesh = zinc.fm.findMeshByDimension(3);
	Elementtemplate elementTemplate = mesh.createElementtemplate();
	EXPECT_TRUE(elementTemplate.isValid());
	EXPECT_EQ(OK, elementTemplate.setElementShapeType(Element::SHAPE_TYPE_CUBE));

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

	Nodeset nodeset = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
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
