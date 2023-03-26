/*
 * Zinc Library Unit Tests
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
#include "zinctestsetup.hpp"

#include <opencmiss/zinc/changemanager.hpp>
#include <opencmiss/zinc/context.hpp>
#include <opencmiss/zinc/element.hpp>
#include <opencmiss/zinc/field.hpp>
#include <opencmiss/zinc/fieldconstant.hpp>
#include <opencmiss/zinc/fieldgroup.hpp>
#include <opencmiss/zinc/fieldlogicaloperators.hpp>
#include <opencmiss/zinc/fieldmodule.hpp>
#include <opencmiss/zinc/node.hpp>
#include <opencmiss/zinc/status.hpp>
#include <opencmiss/zinc/stream.hpp>
#include <opencmiss/zinc/streamregion.hpp>
#include "utilities/testenum.hpp"
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
        cube_si, resourcePath("fieldmodule/two_cubes.exformat").c_str());
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
        resourcePath("fieldmodule/two_cubes.exformat").c_str());
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
	EXPECT_EQ(RESULT_OK, elementTemplate.setElementShapeType(Element::SHAPE_TYPE_CUBE));

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

	EXPECT_EQ(RESULT_OK, mesh.destroyElementsConditional(topHalfIdentifiers));
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
	EXPECT_EQ(RESULT_OK, mesh.destroyAllElements());
	EXPECT_EQ(0, mesh.getSize());
}

// Test destroying elements removes them from all groups, even while change cache is active
TEST(ZincMesh, destroyElementsGroupChangeManager_simple)
{
	ZincTestSetupCpp zinc;

	Mesh mesh = zinc.fm.findMeshByDimension(3);
	Elementtemplate elementTemplate = mesh.createElementtemplate();
	EXPECT_TRUE(elementTemplate.isValid());
	EXPECT_EQ(RESULT_OK, elementTemplate.setElementShapeType(Element::SHAPE_TYPE_CUBE));

	FieldGroup group1 = zinc.fm.createFieldGroup();
	group1.setName("group1");
	group1.setManaged(true);
	MeshGroup group1mesh = group1.createFieldElementGroup(mesh).getMeshGroup();
	EXPECT_TRUE(group1mesh.isValid());

	for (int n = 0; n < 32; ++n)
	{
		Element element = mesh.createElement(n + 1, elementTemplate);
		EXPECT_TRUE(element.isValid());
	}
	EXPECT_EQ(32, mesh.getSize());
	const double one = 1.0;
	EXPECT_EQ(RESULT_OK, group1mesh.addElementsConditional(zinc.fm.createFieldConstant(1, &one)));
	EXPECT_EQ(32, group1mesh.getSize());

	Element element1 = mesh.findElementByIdentifier(1);
	EXPECT_TRUE(element1.isValid());
	EXPECT_TRUE(mesh.containsElement(element1));
	EXPECT_TRUE(group1mesh.containsElement(element1));
	Element element16 = mesh.findElementByIdentifier(16);
	EXPECT_TRUE(element16.isValid());
	EXPECT_TRUE(mesh.containsElement(element16));
	EXPECT_TRUE(group1mesh.containsElement(element16));
	Element element32 = mesh.findElementByIdentifier(32);
	EXPECT_TRUE(element32.isValid());
	EXPECT_TRUE(mesh.containsElement(element32));
	EXPECT_TRUE(group1mesh.containsElement(element32));

	{
		ChangeManager<Fieldmodule> fieldChangeManager(zinc.fm);
		int identifier;

		EXPECT_EQ(RESULT_OK, mesh.destroyElement(element1));
		EXPECT_NE(1, identifier = element1.getIdentifier());  // since invalidated
		EXPECT_FALSE(mesh.containsElement(element1));
		EXPECT_EQ(31, mesh.getSize());
		EXPECT_FALSE(group1mesh.containsElement(element1));  // false as invalidated
		EXPECT_EQ(31, group1mesh.getSize());

		Field cmiss_number = zinc.fm.findFieldByName("cmiss_number");
		EXPECT_TRUE(cmiss_number.isValid());
		const double midIdentifierValue = 16.5;
		FieldConstant midIdentifier = zinc.fm.createFieldConstant(1, &midIdentifierValue);
		EXPECT_TRUE(midIdentifier.isValid());
		FieldLessThan lowerHalfIdentifiers = zinc.fm.createFieldLessThan(cmiss_number, midIdentifier);
		EXPECT_TRUE(midIdentifier.isValid());

		EXPECT_EQ(RESULT_OK, mesh.destroyElementsConditional(lowerHalfIdentifiers));
		EXPECT_NE(16, identifier = element16.getIdentifier());  // since invalidated
		EXPECT_FALSE(mesh.containsElement(element16));
		EXPECT_EQ(16, mesh.getSize());
		EXPECT_FALSE(group1mesh.containsElement(element16));  // false as invalidated
		EXPECT_EQ(16, group1mesh.getSize());

		EXPECT_EQ(RESULT_OK, mesh.destroyAllElements());
		EXPECT_NE(32, identifier = element32.getIdentifier());  // since invalidated
		EXPECT_FALSE(mesh.containsElement(element32));
		EXPECT_EQ(0, mesh.getSize());
		EXPECT_FALSE(group1mesh.containsElement(element32));  // false as invalidated
		EXPECT_EQ(0, group1mesh.getSize());
	}
}

// Test destroying an element with embedded location after reading twice.
// Tests read/merge code properly maintains element to embedded node maps.
TEST(ZincMesh, destroyElement_embeddedLocation)
{
	ZincTestSetupCpp zinc;

    EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(resourcePath("optimisation/fit_line_time.exf").c_str()));
    EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(resourcePath("optimisation/fit_line_time.exf").c_str()));

	Mesh mesh1d = zinc.fm.findMeshByDimension(1);
	Element element1 = mesh1d.findElementByIdentifier(1);
	EXPECT_TRUE(element1.isValid());
	EXPECT_EQ(RESULT_OK, mesh1d.destroyElement(element1));
}

// Test destroying 3D element removes it and all orphaned faces from groups,
// even while change cache is active
TEST(ZincNodeset, destroyElementsGroupChangeManager_cube)
{
	ZincTestSetupCpp zinc;

	EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(resourcePath("fieldmodule/cube.exformat").c_str()));

	// put everything in group1
	FieldGroup group1 = zinc.fm.createFieldGroup();
	group1.setName("group1");
	group1.setManaged(true);
	group1.setSubelementHandlingMode(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL);
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_EQ(1, mesh3d.getSize());
	Element element1 = mesh3d.findElementByIdentifier(1);
	EXPECT_TRUE(element1.isValid());
	MeshGroup group1mesh3d = group1.createFieldElementGroup(mesh3d).getMeshGroup();
	EXPECT_TRUE(group1mesh3d.isValid());
	EXPECT_EQ(RESULT_OK, group1mesh3d.addElement(element1));
	EXPECT_EQ(1, group1mesh3d.getSize());
	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	EXPECT_EQ(6, mesh2d.getSize());
	MeshGroup group1mesh2d = group1.getFieldElementGroup(mesh2d).getMeshGroup();
	EXPECT_EQ(6, group1mesh2d.getSize());
	Mesh mesh1d = zinc.fm.findMeshByDimension(1);
	EXPECT_EQ(12, mesh1d.getSize());
	MeshGroup group1mesh1d = group1.getFieldElementGroup(mesh1d).getMeshGroup();
	EXPECT_EQ(12, group1mesh1d.getSize());
	Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_EQ(8, nodes.getSize());
	NodesetGroup group1nodes = group1.getFieldNodeGroup(nodes).getNodesetGroup();
	EXPECT_EQ(8, group1nodes.getSize());

	{
		ChangeManager<Fieldmodule> fieldChangeManager(zinc.fm);
		int identifier;

		EXPECT_EQ(RESULT_OK, mesh3d.destroyElement(element1));
		EXPECT_NE(1, identifier = element1.getIdentifier());  // since invalidated
		EXPECT_EQ(0, mesh3d.getSize());
		EXPECT_FALSE(mesh3d.containsElement(element1));
		EXPECT_FALSE(group1mesh3d.containsElement(element1));  // false as invalidated
		EXPECT_EQ(0, group1mesh3d.getSize());
		EXPECT_EQ(0, mesh2d.getSize());
		EXPECT_EQ(0, group1mesh2d.getSize());
		EXPECT_EQ(0, mesh1d.getSize());
		EXPECT_EQ(0, group1mesh1d.getSize());
		// nodes are not affected
		EXPECT_EQ(8, nodes.getSize());
		EXPECT_EQ(8, group1nodes.getSize());
	}
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

	EXPECT_EQ(RESULT_OK, nodeset.destroyNodesConditional(topHalfIdentifiers));
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
	EXPECT_EQ(RESULT_OK, nodeset.destroyAllNodes());
	EXPECT_EQ(0, nodeset.getSize());
}

// Test destroying nodes removes them from all groups, even while change cache is active
TEST(ZincNodeset, destroyNodesGroupChangeManager)
{
	ZincTestSetupCpp zinc;

	Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	Nodetemplate nodeTemplate = nodes.createNodetemplate();
	EXPECT_TRUE(nodeTemplate.isValid());

	FieldGroup group1 = zinc.fm.createFieldGroup();
	group1.setName("group1");
	group1.setManaged(true);
	NodesetGroup group1nodes = group1.createFieldNodeGroup(nodes).getNodesetGroup();
	EXPECT_TRUE(group1nodes.isValid());

	for (int n = 0; n < 32; ++n)
	{
		Node node = nodes.createNode(n + 1, nodeTemplate);
		EXPECT_TRUE(node.isValid());
	}
	EXPECT_EQ(32, nodes.getSize());
	const double one = 1.0;
	EXPECT_EQ(RESULT_OK, group1nodes.addNodesConditional(zinc.fm.createFieldConstant(1, &one)));
	EXPECT_EQ(32, group1nodes.getSize());

	Node node1 = nodes.findNodeByIdentifier(1);
	EXPECT_TRUE(node1.isValid());
	EXPECT_TRUE(nodes.containsNode(node1));
	EXPECT_TRUE(group1nodes.containsNode(node1));
	Node node16 = nodes.findNodeByIdentifier(16);
	EXPECT_TRUE(node16.isValid());
	EXPECT_TRUE(nodes.containsNode(node16));
	EXPECT_TRUE(group1nodes.containsNode(node16));
	Node node32 = nodes.findNodeByIdentifier(32);
	EXPECT_TRUE(node32.isValid());
	EXPECT_TRUE(nodes.containsNode(node32));
	EXPECT_TRUE(group1nodes.containsNode(node32));

	{
		ChangeManager<Fieldmodule> fieldChangeManager(zinc.fm);
		int identifier;

		EXPECT_EQ(RESULT_OK, nodes.destroyNode(node1));
		EXPECT_NE(1, identifier = node1.getIdentifier());  // since invalidated
		EXPECT_FALSE(nodes.containsNode(node1));
		EXPECT_EQ(31, nodes.getSize());
		EXPECT_FALSE(group1nodes.containsNode(node1));  // false as invalidated
		EXPECT_EQ(31, group1nodes.getSize());

		Field cmiss_number = zinc.fm.findFieldByName("cmiss_number");
		EXPECT_TRUE(cmiss_number.isValid());
		const double midIdentifierValue = 16.5;
		FieldConstant midIdentifier = zinc.fm.createFieldConstant(1, &midIdentifierValue);
		EXPECT_TRUE(midIdentifier.isValid());
		FieldLessThan lowerHalfIdentifiers = zinc.fm.createFieldLessThan(cmiss_number, midIdentifier);
		EXPECT_TRUE(midIdentifier.isValid());

		EXPECT_EQ(RESULT_OK, nodes.destroyNodesConditional(lowerHalfIdentifiers));
		EXPECT_NE(16, identifier = node16.getIdentifier());  // since invalidated
		EXPECT_FALSE(nodes.containsNode(node16));
		EXPECT_EQ(16, nodes.getSize());
		EXPECT_FALSE(group1nodes.containsNode(node16));  // false as invalidated
		EXPECT_EQ(16, group1nodes.getSize());

		EXPECT_EQ(RESULT_OK, nodes.destroyAllNodes());
		EXPECT_NE(32, identifier = node32.getIdentifier());  // since invalidated
		EXPECT_FALSE(nodes.containsNode(node32));
		EXPECT_EQ(0, nodes.getSize());
		EXPECT_FALSE(group1nodes.containsNode(node32));  // false as invalidated
		EXPECT_EQ(0, group1nodes.getSize());
	}
}

TEST(ZincElement, FaceTypeEnum)
{
	const char *enumNames[10] = { nullptr, "ALL", "ANY_FACE", "NO_FACE", "XI1_0", "XI1_1", "XI2_0", "XI2_1", "XI3_0", "XI3_1" };
	testEnum(10, enumNames, Element::FaceTypeEnumToString, Element::FaceTypeEnumFromString);
}

TEST(ZincElement, PointSamplingModeEnum)
{
	const char *enumNames[6] = { nullptr, "CELL_CENTRES", "CELL_CORNERS", "CELL_POISSON", "SET_LOCATION", "GAUSSIAN_QUADRATURE" };
	testEnum(6, enumNames, Element::PointSamplingModeEnumToString, Element::PointSamplingModeEnumFromString);
}

TEST(ZincElement, QuadratureRuleEnum)
{
	const char *enumNames[3] = { nullptr, "GAUSSIAN", "MIDPOINT" };
	testEnum(3, enumNames, Element::QuadratureRuleEnumToString, Element::QuadratureRuleEnumFromString);
}

TEST(ZincElement, ShapeTypeEnum)
{
	const char *enumNames[9] = { nullptr, "LINE", "SQUARE", "TRIANGLE", "CUBE", "TETRAHEDRON", "WEDGE12", "WEDGE13", "WEDGE23" };
	testEnum(9, enumNames, Element::ShapeTypeEnumToString, Element::ShapeTypeEnumFromString);
}

TEST(ZincNode, ValueLabelEnum)
{
	const char *enumNames[9] = { nullptr, "VALUE", "D_DS1", "D_DS2", "D2_DS1DS2", "D_DS3", "D2_DS1DS3", "D2_DS2DS3", "D3_DS1DS2DS3" };
	testEnum(9, enumNames, Node::ValueLabelEnumToString, Node::ValueLabelEnumFromString);
}
