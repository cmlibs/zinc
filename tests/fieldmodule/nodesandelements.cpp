/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <opencmiss/zinc/core.h>
#include <opencmiss/zinc/context.h>
#include <opencmiss/zinc/element.h>
#include <opencmiss/zinc/field.h>
#include <opencmiss/zinc/fieldmodule.h>
#include <opencmiss/zinc/fieldfiniteelement.h>
#include <opencmiss/zinc/node.h>
#include <opencmiss/zinc/region.h>
#include <opencmiss/zinc/status.h>
#include <opencmiss/zinc/stream.h>
#include <opencmiss/zinc/streamregion.h>

#include <opencmiss/zinc/context.hpp>
#include <opencmiss/zinc/element.hpp>
#include <opencmiss/zinc/field.hpp>
#include <opencmiss/zinc/fieldconstant.hpp>
#include <opencmiss/zinc/fieldlogicaloperators.hpp>
#include <opencmiss/zinc/fieldmodule.hpp>
#include <opencmiss/zinc/node.hpp>
#include <opencmiss/zinc/status.hpp>
#include <opencmiss/zinc/stream.hpp>
#include <opencmiss/zinc/streamregion.hpp>
#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

#include "test_resources.h"


TEST(nodes_elements_identifier, set_identifier)
{
	ZincTestSetup zinc;

	cmzn_region_id cube_region = cmzn_region_create_child(zinc.root_region, "cube");
	cmzn_streaminformation_id cube_si = cmzn_region_create_streaminformation_region(
		cube_region);
	EXPECT_NE(static_cast<cmzn_streaminformation *>(0), cube_si);

	cmzn_streamresource_id cube_sr = cmzn_streaminformation_create_streamresource_file(
		cube_si, TestResources::getLocation(TestResources::FIELDMODULE_TWO_CUBES_RESOURCE));
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

	cmzn_fieldmodule_id tmpFm = cmzn_nodeset_get_fieldmodule(nodeset);
	EXPECT_TRUE(cmzn_fieldmodule_match(cubeFm, tmpFm));
	cmzn_fieldmodule_destroy(&tmpFm);

	cmzn_node_id node1 = cmzn_nodeset_find_node_by_identifier(nodeset, 1);
	EXPECT_NE(static_cast<cmzn_node *>(0), node1);

	cmzn_nodeset_id tmpNodeset = cmzn_node_get_nodeset(node1);
	EXPECT_TRUE(cmzn_nodeset_match(nodeset, tmpNodeset));
	cmzn_nodeset_destroy(&tmpNodeset);

	EXPECT_EQ(CMZN_OK, result = cmzn_node_set_identifier(node1, 1)); // can always set to current identifier
	EXPECT_EQ(CMZN_ERROR_ALREADY_EXISTS, result = cmzn_node_set_identifier(node1, 3));
	EXPECT_EQ(CMZN_OK, result = cmzn_node_set_identifier(node1, 13));

	cmzn_mesh_id mesh = cmzn_fieldmodule_find_mesh_by_dimension(cubeFm, 3);
	EXPECT_NE(static_cast<cmzn_mesh *>(0), mesh);

	tmpFm = cmzn_mesh_get_fieldmodule(mesh);
	EXPECT_TRUE(cmzn_fieldmodule_match(cubeFm, tmpFm));
	cmzn_fieldmodule_destroy(&tmpFm);

	cmzn_element_id element1 = cmzn_mesh_find_element_by_identifier(mesh, 1);
	EXPECT_NE(static_cast<cmzn_element *>(0), element1);

	cmzn_mesh_id tmpMesh = cmzn_element_get_mesh(element1);
	EXPECT_TRUE(cmzn_mesh_match(mesh, tmpMesh));
	cmzn_mesh_destroy(&tmpMesh);

	EXPECT_EQ(CMZN_OK, result = cmzn_element_set_identifier(element1, 1)); // can always set to current identifier
	EXPECT_EQ(CMZN_ERROR_ALREADY_EXISTS, result = cmzn_element_set_identifier(element1, 2));
	EXPECT_EQ(CMZN_OK, result = cmzn_element_set_identifier(element1, 3));

	cmzn_element_destroy(&element1);
	cmzn_mesh_destroy(&mesh);
	cmzn_fieldmodule_destroy(&cubeFm);
	cmzn_nodeset_destroy(&nodeset);
	cmzn_node_destroy(&node1);

	cmzn_region_destroy(&cube_region);
}

TEST(ZincNodesElements, setIdentifier)
{
	ZincTestSetupCpp zinc;

	Region cubeRegion = zinc.root_region.createChild("cube");
	StreaminformationRegion cubeSi = cubeRegion.createStreaminformationRegion();
	EXPECT_TRUE(cubeSi.isValid());

	Streamresource cubeSr = cubeSi.createStreamresourceFile(
		TestResources::getLocation(TestResources::FIELDMODULE_TWO_CUBES_RESOURCE));
	EXPECT_TRUE(cubeSr.isValid());

	// test casting of stream resources
	EXPECT_TRUE(cubeSr.castFile().isValid());
	EXPECT_FALSE(cubeSr.castMemory().isValid());

	int result;
	EXPECT_EQ(OK, result = cubeRegion.read(cubeSi));

	Fieldmodule cubeFm = cubeRegion.getFieldmodule();
	EXPECT_TRUE(cubeFm.isValid());

	Nodeset nodeset = cubeFm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodeset.isValid());

	EXPECT_EQ(cubeFm, nodeset.getFieldmodule());

	Node node1 = nodeset.findNodeByIdentifier(1);
	EXPECT_TRUE(node1.isValid());

	EXPECT_EQ(nodeset, node1.getNodeset());

	EXPECT_EQ(OK, result = node1.setIdentifier(1)); // can always set to current identifier
	EXPECT_EQ(ERROR_ALREADY_EXISTS, result = node1.setIdentifier(3));
	EXPECT_EQ(OK, result = node1.setIdentifier(13));

	Mesh mesh = cubeFm.findMeshByDimension(3);
	EXPECT_TRUE(mesh.isValid());

	EXPECT_EQ(cubeFm, mesh.getFieldmodule());

	Element element1 = mesh.findElementByIdentifier(1);
	EXPECT_TRUE(element1.isValid());

	EXPECT_EQ(mesh, element1.getMesh());

	EXPECT_EQ(OK, result = element1.setIdentifier(1)); // can always set to current identifier
	EXPECT_EQ(ERROR_ALREADY_EXISTS, result = element1.setIdentifier(2));
	EXPECT_EQ(OK, result = element1.setIdentifier(3));
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

TEST(ZincElementiterator, invalidation)
{
	ZincTestSetupCpp zinc;

	Region childRegion = zinc.root_region.createChild("temp");
	Fieldmodule fm = childRegion.getFieldmodule();

	Mesh mesh = fm.findMeshByDimension(3);
	Elementtemplate elementTemplate = mesh.createElementtemplate();
	EXPECT_TRUE(elementTemplate.isValid());
	EXPECT_EQ(OK, elementTemplate.setElementShapeType(Element::SHAPE_TYPE_CUBE));

	Element e1 = mesh.createElement(1, elementTemplate);
	EXPECT_TRUE(e1.isValid());
	Element e2 = mesh.createElement(2, elementTemplate);
	EXPECT_TRUE(e2.isValid());
	Element e3 = mesh.createElement(3, elementTemplate);
	EXPECT_TRUE(e3.isValid());

	Elementiterator iter;
	Element e;

	// test that creating a new element safely invalidates iterator
	iter = mesh.createElementiterator();
	EXPECT_TRUE(iter.isValid());
	e = iter.next();
	EXPECT_EQ(e1, e);
	e = iter.next();
	EXPECT_EQ(e2, e);
	EXPECT_EQ(3, mesh.getSize());
	Element e4 = mesh.createElement(4, elementTemplate);
	EXPECT_TRUE(e4.isValid());
	EXPECT_EQ(4, mesh.getSize());
	e = iter.next();
	EXPECT_FALSE(e.isValid());

	// test that removing an element safely invalidates iterator
	elementTemplate = Elementtemplate();
	fm = Fieldmodule();
	iter = mesh.createElementiterator();
	EXPECT_TRUE(iter.isValid());
	e = iter.next();
	EXPECT_EQ(e1, e);
	e = iter.next();
	EXPECT_EQ(e2, e);
	EXPECT_EQ(OK, mesh.destroyElement(e4));
	EXPECT_EQ(3, mesh.getSize());
	Mesh tmpMesh = e4.getMesh();
	EXPECT_FALSE(tmpMesh.isValid());
	e = iter.next();
	EXPECT_FALSE(e.isValid());

	// test that renumbering an element safely invalidates iterator
	iter = mesh.createElementiterator();
	EXPECT_TRUE(iter.isValid());
	e = iter.next();
	EXPECT_EQ(e1, e);
	e = iter.next();
	EXPECT_EQ(e2, e);
	EXPECT_EQ(OK, e3.setIdentifier(5));
	e = iter.next();
	EXPECT_FALSE(e.isValid());

	// test that destroying child region safely invalidates iterator
	iter = mesh.createElementiterator();
	EXPECT_TRUE(iter.isValid());
	e = iter.next();
	EXPECT_EQ(e1, e);
	e = iter.next();
	EXPECT_EQ(e2, e);
	EXPECT_EQ(OK, zinc.root_region.removeChild(childRegion));
	childRegion = Region(); // clear handle so it can be destroyed
	mesh = Mesh(); // clear handle
	e = iter.next();
	EXPECT_FALSE(e.isValid());
	tmpMesh = e3.getMesh();
	EXPECT_FALSE(tmpMesh.isValid());
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

TEST(ZincNodeiterator, invalidation)
{
	ZincTestSetupCpp zinc;

	Region childRegion = zinc.root_region.createChild("temp");
	Fieldmodule fm = childRegion.getFieldmodule();

	Nodeset nodeset = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	Nodetemplate nodeTemplate = nodeset.createNodetemplate();
	EXPECT_TRUE(nodeTemplate.isValid());

	Node n1 = nodeset.createNode(1, nodeTemplate);
	EXPECT_TRUE(n1.isValid());
	Node n2 = nodeset.createNode(2, nodeTemplate);
	EXPECT_TRUE(n2.isValid());
	Node n3 = nodeset.createNode(3, nodeTemplate);
	EXPECT_TRUE(n3.isValid());

	Nodeiterator iter;
	Node n;

	// test that creating a new node safely invalidates iterator
	iter = nodeset.createNodeiterator();
	EXPECT_TRUE(iter.isValid());
	n = iter.next();
	EXPECT_EQ(n1, n);
	n = iter.next();
	EXPECT_EQ(n2, n);
	EXPECT_EQ(3, nodeset.getSize());
	Node n4 = nodeset.createNode(4, nodeTemplate);
	EXPECT_TRUE(n4.isValid());
	EXPECT_EQ(4, nodeset.getSize());
	n = iter.next();
	EXPECT_FALSE(n.isValid());

	// test that removing a node safely invalidates iterator
	nodeTemplate = Nodetemplate();
	fm = Fieldmodule();
	iter = nodeset.createNodeiterator();
	EXPECT_TRUE(iter.isValid());
	n = iter.next();
	EXPECT_EQ(n1, n);
	n = iter.next();
	EXPECT_EQ(n2, n);
	EXPECT_EQ(OK, nodeset.destroyNode(n4));
	EXPECT_EQ(3, nodeset.getSize());
	Nodeset tmpNodeset = n4.getNodeset();
	EXPECT_FALSE(tmpNodeset.isValid());
	n = iter.next();
	EXPECT_FALSE(n.isValid());

	// test that renumbering a node safely invalidates iterator
	iter = nodeset.createNodeiterator();
	EXPECT_TRUE(iter.isValid());
	n = iter.next();
	EXPECT_EQ(n1, n);
	n = iter.next();
	EXPECT_EQ(n2, n);
	EXPECT_EQ(OK, n3.setIdentifier(5));
	n = iter.next();
	EXPECT_FALSE(n.isValid());

	// test that destroying child region safely invalidates iterator
	iter = nodeset.createNodeiterator();
	EXPECT_TRUE(iter.isValid());
	n = iter.next();
	EXPECT_EQ(n1, n);
	n = iter.next();
	EXPECT_EQ(n2, n);
	EXPECT_EQ(OK, zinc.root_region.removeChild(childRegion));
	childRegion = Region(); // clear handle so it can be destroyed
	nodeset = Nodeset(); // clear handle
	n = iter.next();
	EXPECT_FALSE(n.isValid());
	tmpNodeset = n3.getNodeset();
	EXPECT_FALSE(tmpNodeset.isValid());
}

TEST(ZincMesh, destroyElements)
{
	ZincTestSetupCpp zinc;

	Mesh mesh = zinc.fm.findMeshByDimension(3);
	Elementtemplate elementTemplate = mesh.createElementtemplate();
	EXPECT_TRUE(elementTemplate.isValid());
	EXPECT_EQ(OK, elementTemplate.setElementShapeType(Element::SHAPE_TYPE_CUBE));

	Element element[32];
	for (int e = 0; e < 32; ++e)
	{
		element[e] = mesh.createElement(e + 1, elementTemplate);
		EXPECT_TRUE(element[e].isValid());
	}
	EXPECT_EQ(32, mesh.getSize());

	Field cmiss_number = zinc.fm.findFieldByName("cmiss_number");
	EXPECT_TRUE(cmiss_number.isValid());
	const double midIdentifierValue = 16.5;
	FieldConstant midIdentifier = zinc.fm.createFieldConstant(1, &midIdentifierValue);
	EXPECT_TRUE(midIdentifier.isValid());
	FieldGreaterThan topHalfIdentifiers = zinc.fm.createFieldGreaterThan(cmiss_number, midIdentifier);
	EXPECT_TRUE(midIdentifier.isValid());

	EXPECT_EQ(OK, mesh.destroyElementsConditional(topHalfIdentifiers));
	EXPECT_EQ(16, mesh.getSize());
	Mesh tmpMesh = element[16].getMesh();
	EXPECT_FALSE(tmpMesh.isValid());

	Elementiterator iter = mesh.createElementiterator();
	Element el;
	for (int e = 0; e < 16; ++e)
	{
		el = iter.next();
		EXPECT_EQ(element[e], el);
	}
	EXPECT_EQ(OK, mesh.destroyAllElements());
	EXPECT_EQ(0, mesh.getSize());
}

TEST(ZincNodeset, destroyNodes)
{
	ZincTestSetupCpp zinc;

	Nodeset nodeset = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	Nodetemplate nodeTemplate = nodeset.createNodetemplate();
	EXPECT_TRUE(nodeTemplate.isValid());

	Node node[32];
	for (int n = 0; n < 32; ++n)
	{
		node[n] = nodeset.createNode(n + 1, nodeTemplate);
		EXPECT_TRUE(node[n].isValid());
	}
	EXPECT_EQ(32, nodeset.getSize());

	Field cmiss_number = zinc.fm.findFieldByName("cmiss_number");
	EXPECT_TRUE(cmiss_number.isValid());
	const double midIdentifierValue = 16.5;
	FieldConstant midIdentifier = zinc.fm.createFieldConstant(1, &midIdentifierValue);
	EXPECT_TRUE(midIdentifier.isValid());
	FieldGreaterThan topHalfIdentifiers = zinc.fm.createFieldGreaterThan(cmiss_number, midIdentifier);
	EXPECT_TRUE(midIdentifier.isValid());

	EXPECT_EQ(OK, nodeset.destroyNodesConditional(topHalfIdentifiers));
	EXPECT_EQ(16, nodeset.getSize());
	Nodeset tmpNodeset = node[16].getNodeset();
	EXPECT_FALSE(tmpNodeset.isValid());

	Nodeiterator iter = nodeset.createNodeiterator();
	Node el;
	for (int n = 0; n < 16; ++n)
	{
		el = iter.next();
		EXPECT_EQ(node[n], el);
	}
	EXPECT_EQ(OK, nodeset.destroyAllNodes());
	EXPECT_EQ(0, nodeset.getSize());
}
