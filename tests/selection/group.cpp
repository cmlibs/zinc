/*
 * Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <cmlibs/zinc/core.h>
#include <cmlibs/zinc/element.h>
#include <cmlibs/zinc/field.h>
#include <cmlibs/zinc/fieldcache.h>
#include <cmlibs/zinc/fieldcomposite.h>
#include <cmlibs/zinc/fieldconstant.h>
#include <cmlibs/zinc/fieldfiniteelement.h>
#include <cmlibs/zinc/fieldgroup.h>
#include <cmlibs/zinc/fieldlogicaloperators.h>
#include <cmlibs/zinc/fieldmeshoperators.hpp>
#include <cmlibs/zinc/fieldnodesetoperators.hpp>
#include <cmlibs/zinc/mesh.h>
#include <cmlibs/zinc/node.h>
#include <cmlibs/zinc/region.h>

#include <cmlibs/zinc/element.hpp>
#include <cmlibs/zinc/field.hpp>
#include <cmlibs/zinc/fieldcache.hpp>
#include <cmlibs/zinc/fieldcomposite.hpp>
#include <cmlibs/zinc/fieldconstant.hpp>
#include <cmlibs/zinc/fieldfiniteelement.hpp>
#include <cmlibs/zinc/fieldgroup.hpp>
#include <cmlibs/zinc/fieldlogicaloperators.hpp>
#include <cmlibs/zinc/mesh.hpp>
#include <cmlibs/zinc/node.hpp>
#include <cmlibs/zinc/region.hpp>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

#include "test_resources.h"

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

TEST(cmzn_fieldgroup, add_remove_nodes)
{
	ZincTestSetup zinc;
	int result;

	cmzn_nodeset_id nodes = cmzn_fieldmodule_find_nodeset_by_field_domain_type(zinc.fm, CMZN_FIELD_DOMAIN_TYPE_NODES);
	cmzn_region_id childRegion = cmzn_region_create_child(zinc.root_region, "child");
	cmzn_fieldmodule_id childFm = cmzn_region_get_fieldmodule(childRegion);
	addSomeNodes(childFm);
	cmzn_nodeset_id childNodes = cmzn_fieldmodule_find_nodeset_by_field_domain_type(childFm, CMZN_FIELD_DOMAIN_TYPE_NODES);
	cmzn_node_id childNode = cmzn_nodeset_find_node_by_identifier(childNodes, 2);
	EXPECT_NE(static_cast<cmzn_node_id>(0), childNode);
	cmzn_nodeset_id childDatapoints = cmzn_fieldmodule_find_nodeset_by_field_domain_type(childFm, CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS);

	cmzn_field_id groupField = cmzn_fieldmodule_create_field_group(zinc.fm);
	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_name(groupField, "group"));
	cmzn_field_group_id group = cmzn_field_cast_group(groupField);
	EXPECT_NE(static_cast<cmzn_field_group_id>(0), group);

	// test find/create nodeset group for a subregion
	cmzn_nodeset_group_id nodesetGroup = 0;
	cmzn_nodeset_group_id tmpNodesetGroup = 0;
	EXPECT_EQ(static_cast<cmzn_nodeset_group_id>(0), cmzn_field_group_get_nodeset_group(group, childNodes));
	EXPECT_NE(static_cast<cmzn_nodeset_group_id>(0), nodesetGroup = cmzn_field_group_create_nodeset_group(group, childNodes));
	EXPECT_EQ(static_cast<cmzn_nodeset_group_id>(0), cmzn_field_group_create_nodeset_group(group, childNodes));
	EXPECT_EQ(nodesetGroup, tmpNodesetGroup = cmzn_field_group_get_nodeset_group(group, childNodes));
	cmzn_nodeset_group_destroy(&tmpNodesetGroup);
	EXPECT_EQ(nodesetGroup, tmpNodesetGroup = cmzn_field_group_get_or_create_nodeset_group(group, childNodes));
	cmzn_nodeset_group_destroy(&tmpNodesetGroup);
	char* tmpName = 0;
	EXPECT_STREQ("group.nodes", tmpName = cmzn_nodeset_get_name(cmzn_nodeset_group_base_cast(nodesetGroup)));
	cmzn_deallocate(tmpName);
	EXPECT_NE(static_cast<cmzn_nodeset_group_id>(0), tmpNodesetGroup = cmzn_field_group_get_or_create_nodeset_group(group, childDatapoints));
	cmzn_nodeset_group_destroy(&tmpNodesetGroup);
	cmzn_nodeset_id tmpNodeset = 0;
	EXPECT_EQ(cmzn_nodeset_group_base_cast(nodesetGroup), tmpNodeset = cmzn_fieldmodule_find_nodeset_by_name(childFm, "group.nodes"));
	cmzn_nodeset_destroy(&tmpNodeset);

	// test contains/add
	EXPECT_FALSE(cmzn_nodeset_contains_node(cmzn_nodeset_group_base_cast(nodesetGroup), childNode));
	EXPECT_EQ(CMZN_OK, cmzn_nodeset_group_add_node(nodesetGroup, childNode));
	EXPECT_TRUE(cmzn_nodeset_contains_node(cmzn_nodeset_group_base_cast(nodesetGroup), childNode));

	// test child group created properly, and that we find non-empty subgroup
	cmzn_field_group_id childGroup = 0;
	EXPECT_NE(static_cast<cmzn_field_group_id>(0), childGroup = cmzn_field_group_get_subregion_field_group(group, childRegion));
	cmzn_field_group_id tmpChildGroup = 0;
	EXPECT_NE(static_cast<cmzn_field_group_id>(0), tmpChildGroup = cmzn_field_group_get_or_create_subregion_field_group(group, childRegion));
	cmzn_field_group_destroy(&tmpChildGroup);
	EXPECT_STREQ("group", tmpName = cmzn_field_get_name(cmzn_field_group_base_cast(childGroup)));
	cmzn_deallocate(tmpName);
	cmzn_field_group_id nonEmptyGroup = 0;
	EXPECT_EQ(childGroup, nonEmptyGroup = cmzn_field_group_get_first_non_empty_subregion_field_group(group));
	cmzn_field_group_destroy(&nonEmptyGroup);

	EXPECT_EQ(nodesetGroup, tmpNodesetGroup = cmzn_field_group_get_nodeset_group(childGroup, childNodes));
	cmzn_nodeset_group_destroy(&tmpNodesetGroup);
	cmzn_field_group_id getGroup = cmzn_nodeset_group_get_field_group(nodesetGroup);
	EXPECT_EQ(childGroup, getGroup);
	cmzn_field_group_destroy(&getGroup);
	cmzn_field_group_destroy(&childGroup);
	// test can't get nodeset group for parent region nodes
	EXPECT_EQ(static_cast<cmzn_nodeset_group_id>(0), cmzn_field_group_get_nodeset_group(childGroup, nodes));

	// test remove/contains
	EXPECT_EQ(CMZN_OK, cmzn_nodeset_group_remove_node(nodesetGroup, childNode));
	EXPECT_FALSE(cmzn_nodeset_contains_node(cmzn_nodeset_group_base_cast(nodesetGroup), childNode));

	// test for empty groups
	EXPECT_EQ(static_cast<cmzn_field_group_id>(0), nonEmptyGroup = cmzn_field_group_get_first_non_empty_subregion_field_group(group));
	cmzn_nodeset_group_destroy(&nodesetGroup);  // otherwise child group remains
	EXPECT_EQ(CMZN_OK, cmzn_field_group_remove_empty_subgroups(group));
	EXPECT_EQ(static_cast<cmzn_field_group_id>(0), childGroup = cmzn_field_group_get_subregion_field_group(group, childRegion));

	cmzn_field_group_destroy(&group);
	cmzn_field_destroy(&groupField);

	cmzn_nodeset_destroy(&childDatapoints);
	cmzn_node_destroy(&childNode);
	cmzn_nodeset_destroy(&childNodes);
	cmzn_nodeset_destroy(&nodes);
	cmzn_fieldmodule_destroy(&childFm);
	cmzn_region_destroy(&childRegion);
}

TEST(ZincFieldGroup, add_remove_nodes)
{
	ZincTestSetupCpp zinc;
	int result;

	Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	Region childRegion = zinc.root_region.createChild("child");
	Fieldmodule childFm = childRegion.getFieldmodule();
	addSomeNodes(childFm.getId());
	Nodeset childNodes = childFm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(childNodes.isValid());
	Node childNode = childNodes.findNodeByIdentifier(2);
	EXPECT_TRUE(childNode.isValid());
	Nodeset childDatapoints = childFm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_DATAPOINTS);

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());
	EXPECT_EQ(RESULT_OK, result = group.setName("group"));

	// test find/create nodeset group for a subregion
	NodesetGroup nodesetGroup;
	EXPECT_FALSE(group.getNodesetGroup(childNodes).isValid());
	EXPECT_TRUE((nodesetGroup = group.createNodesetGroup(childNodes)).isValid());
	EXPECT_FALSE(group.createNodesetGroup(childNodes).isValid());
	EXPECT_TRUE(group.getNodesetGroup(childNodes).isValid());
	EXPECT_TRUE(group.getOrCreateNodesetGroup(childNodes).isValid());
	char* tmpName = 0;
	EXPECT_STREQ("group.nodes", tmpName = nodesetGroup.getName());
	cmzn_deallocate(tmpName);
	EXPECT_TRUE(group.getOrCreateNodesetGroup(childDatapoints).isValid());
	EXPECT_EQ(nodesetGroup, childFm.findNodesetByName("group.nodes"));

	// test contains/add
	EXPECT_FALSE(nodesetGroup.containsNode(childNode));
	EXPECT_EQ(RESULT_OK, nodesetGroup.addNode(childNode));
	EXPECT_TRUE(nodesetGroup.containsNode(childNode));

	// test child group created properly, and that we find non-empty subgroup
	FieldGroup childGroup;
	EXPECT_TRUE((childGroup = group.getSubregionFieldGroup(childRegion)).isValid());
	EXPECT_TRUE(group.getOrCreateSubregionFieldGroup(childRegion).isValid());
	EXPECT_STREQ("group", tmpName = childGroup.getName());
	cmzn_deallocate(tmpName);
	FieldGroup nonEmptyGroup;
	EXPECT_EQ(childGroup, nonEmptyGroup = group.getFirstNonEmptySubregionFieldGroup());

	EXPECT_EQ(nodesetGroup, childGroup.getNodesetGroup(childNodes));
	EXPECT_EQ(childGroup, nodesetGroup.getFieldGroup());
	// test can't get nodeset group for parent region nodes
	EXPECT_FALSE(childGroup.getNodesetGroup(nodes).isValid());

	// test remove/contains
	EXPECT_EQ(RESULT_OK, nodesetGroup.removeNode(childNode));
	EXPECT_FALSE(nodesetGroup.containsNode(childNode));

	// test for empty groups
	childGroup = FieldGroup();  // so not rediscovered by name
	EXPECT_FALSE((nonEmptyGroup = group.getFirstNonEmptySubregionFieldGroup()).isValid());
	nodesetGroup = NodesetGroup();  // otherwise child group remains
	EXPECT_EQ(OK, group.removeEmptySubgroups());
	EXPECT_FALSE((childGroup = group.getSubregionFieldGroup(childRegion)).isValid());
}

// test that FieldGroup lives as long as any NodesetGroup made for it
TEST(ZincFieldGroup, lifespan_nodeset_group)
{
	ZincTestSetupCpp zinc;

	Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());
	EXPECT_EQ(RESULT_OK, group.setName("group"));
	cmzn_field_id field_id = group.getId();
	EXPECT_NE(nullptr, field_id);

	NodesetGroup nodesetGroup = group.createNodesetGroup(nodes);
	EXPECT_TRUE(nodesetGroup.isValid());
	// the following releases the handle but should not destroy group
	group = FieldGroup();
	EXPECT_EQ(nullptr, group.getId());

	group = nodesetGroup.getFieldGroup();
	EXPECT_EQ(field_id, group.getId());
}

TEST(cmzn_fieldgroup, add_remove_elements)
{
	ZincTestSetup zinc;
	int result;

	cmzn_mesh_id mesh3d = cmzn_fieldmodule_find_mesh_by_dimension(zinc.fm, 3);
	cmzn_region_id childRegion = cmzn_region_create_child(zinc.root_region, "child");
	cmzn_fieldmodule_id childFm = cmzn_region_get_fieldmodule(childRegion);
	addSomeElements(childFm);
	cmzn_mesh_id childMesh3d = cmzn_fieldmodule_find_mesh_by_dimension(childFm, 3);
	cmzn_element_id childElement = cmzn_mesh_find_element_by_identifier(childMesh3d, 2);
	EXPECT_NE(static_cast<cmzn_element_id>(0), childElement);
	cmzn_mesh_id childMesh2d = cmzn_fieldmodule_find_mesh_by_dimension(childFm, 2);

	cmzn_field_id groupField = cmzn_fieldmodule_create_field_group(zinc.fm);
	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_name(groupField, "group"));
	cmzn_field_group_id group = cmzn_field_cast_group(groupField);
	EXPECT_NE(static_cast<cmzn_field_group_id>(0), group);

	// test find/create mesh group for a subregion
	cmzn_mesh_group_id meshGroup = 0;
	cmzn_mesh_group_id tmpMeshGroup = 0;
	EXPECT_EQ(static_cast<cmzn_mesh_group_id>(0), cmzn_field_group_get_mesh_group(group, childMesh3d));
	EXPECT_NE(static_cast<cmzn_mesh_group_id>(0), meshGroup = cmzn_field_group_create_mesh_group(group, childMesh3d));
	EXPECT_EQ(static_cast<cmzn_mesh_group_id>(0), cmzn_field_group_create_mesh_group(group, childMesh3d));
	EXPECT_EQ(meshGroup, tmpMeshGroup = cmzn_field_group_get_mesh_group(group, childMesh3d));
	cmzn_mesh_group_destroy(&tmpMeshGroup);
	EXPECT_EQ(meshGroup, tmpMeshGroup = cmzn_field_group_get_or_create_mesh_group(group, childMesh3d));
	cmzn_mesh_group_destroy(&tmpMeshGroup);
	char* tmpName = 0;
	EXPECT_STREQ("group.mesh3d", tmpName = cmzn_mesh_get_name(cmzn_mesh_group_base_cast(meshGroup)));
	cmzn_deallocate(tmpName);
	EXPECT_NE(static_cast<cmzn_mesh_group_id>(0), tmpMeshGroup = cmzn_field_group_get_or_create_mesh_group(group, childMesh2d));
	cmzn_mesh_group_destroy(&tmpMeshGroup);
	cmzn_mesh_id tmpMesh = 0;
	EXPECT_EQ(cmzn_mesh_group_base_cast(meshGroup), tmpMesh = cmzn_fieldmodule_find_mesh_by_name(childFm, "group.mesh3d"));
	cmzn_mesh_destroy(&tmpMesh);

	// test contains/add
	EXPECT_FALSE(cmzn_mesh_contains_element(cmzn_mesh_group_base_cast(meshGroup), childElement));
	EXPECT_EQ(CMZN_OK, cmzn_mesh_group_add_element(meshGroup, childElement));
	EXPECT_TRUE(cmzn_mesh_contains_element(cmzn_mesh_group_base_cast(meshGroup), childElement));

	// test child group created properly, and that we find non-empty subgroup
	cmzn_field_group_id childGroup = 0;
	EXPECT_NE(static_cast<cmzn_field_group_id>(0), childGroup = cmzn_field_group_get_subregion_field_group(group, childRegion));
	cmzn_field_group_id tmpChildGroup = 0;
	EXPECT_NE(static_cast<cmzn_field_group_id>(0), tmpChildGroup = cmzn_field_group_get_or_create_subregion_field_group(group, childRegion));
	cmzn_field_group_destroy(&tmpChildGroup);
	EXPECT_STREQ("group", tmpName = cmzn_field_get_name(cmzn_field_group_base_cast(childGroup)));
	cmzn_deallocate(tmpName);
	cmzn_field_group_id nonEmptyGroup = 0;
	EXPECT_EQ(childGroup, nonEmptyGroup = cmzn_field_group_get_first_non_empty_subregion_field_group(group));
	cmzn_field_group_destroy(&nonEmptyGroup);

	EXPECT_EQ(meshGroup, tmpMeshGroup = cmzn_field_group_get_mesh_group(childGroup, childMesh3d));
	cmzn_mesh_group_destroy(&tmpMeshGroup);
	cmzn_field_group_id getGroup = cmzn_mesh_group_get_field_group(meshGroup);
	EXPECT_EQ(childGroup, getGroup);
	cmzn_field_group_destroy(&getGroup);
	cmzn_field_group_destroy(&childGroup);
	// test can't get mesh group for parent region mesh
	EXPECT_EQ(static_cast<cmzn_mesh_group_id>(0), cmzn_field_group_get_mesh_group(childGroup, mesh3d));

	// test remove/contains
	EXPECT_EQ(CMZN_OK, cmzn_mesh_group_remove_element(meshGroup, childElement));
	EXPECT_FALSE(cmzn_mesh_contains_element(cmzn_mesh_group_base_cast(meshGroup), childElement));

	// test for empty groups
	EXPECT_EQ(static_cast<cmzn_field_group_id>(0), nonEmptyGroup = cmzn_field_group_get_first_non_empty_subregion_field_group(group));
	cmzn_mesh_group_destroy(&meshGroup);  // otherwise child group remains
	EXPECT_EQ(CMZN_OK, cmzn_field_group_remove_empty_subgroups(group));
	EXPECT_EQ(static_cast<cmzn_field_group_id>(0), childGroup = cmzn_field_group_get_subregion_field_group(group, childRegion));

	cmzn_field_group_destroy(&group);
	cmzn_field_destroy(&groupField);

	cmzn_mesh_destroy(&childMesh2d);
	cmzn_element_destroy(&childElement);
	cmzn_mesh_destroy(&childMesh3d);
	cmzn_mesh_destroy(&mesh3d);
	cmzn_fieldmodule_destroy(&childFm);
	cmzn_region_destroy(&childRegion);
}

TEST(ZincFieldGroup, add_remove_elements)
{
	ZincTestSetupCpp zinc;
	int result;

	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	Region childRegion = zinc.root_region.createChild("child");
	Fieldmodule childFm = childRegion.getFieldmodule();
	addSomeElements(childFm.getId());
	Mesh childMesh3d = childFm.findMeshByDimension(3);
	EXPECT_TRUE(childMesh3d.isValid());
	Element childElement = childMesh3d.findElementByIdentifier(2);
	EXPECT_TRUE(childElement.isValid());
	Mesh childMesh2d = childFm.findMeshByDimension(2);

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());
	EXPECT_EQ(CMZN_OK, result = group.setName("group"));

	// test find/create mesh group for a subregion
	MeshGroup meshGroup;
	EXPECT_FALSE(group.getMeshGroup(childMesh3d).isValid());
	EXPECT_TRUE((meshGroup = group.createMeshGroup(childMesh3d)).isValid());
	EXPECT_FALSE(group.createMeshGroup(childMesh3d).isValid());
	EXPECT_TRUE(group.getMeshGroup(childMesh3d).isValid());
	EXPECT_TRUE(group.getOrCreateMeshGroup(childMesh3d).isValid());
	char* tmpName = 0;
	EXPECT_STREQ("group.mesh3d", tmpName = meshGroup.getName());
	cmzn_deallocate(tmpName);
	EXPECT_TRUE(group.getOrCreateMeshGroup(childMesh2d).isValid());
	EXPECT_EQ(meshGroup, childFm.findMeshByName("group.mesh3d"));

	// test contains/add
	EXPECT_FALSE(meshGroup.containsElement(childElement));
	EXPECT_EQ(RESULT_OK, meshGroup.addElement(childElement));
	EXPECT_TRUE(meshGroup.containsElement(childElement));

	// test child group created properly, and that we find non-empty subgroup
	FieldGroup childGroup;
	EXPECT_TRUE((childGroup = group.getSubregionFieldGroup(childRegion)).isValid());
	EXPECT_TRUE(group.getOrCreateSubregionFieldGroup(childRegion).isValid());
	EXPECT_STREQ("group", tmpName = childGroup.getName());
	cmzn_deallocate(tmpName);
	FieldGroup nonEmptyGroup;
	EXPECT_EQ(childGroup, nonEmptyGroup = group.getFirstNonEmptySubregionFieldGroup());

	EXPECT_EQ(meshGroup, childGroup.getMeshGroup(childMesh3d));
	EXPECT_EQ(childGroup, meshGroup.getFieldGroup());
	// test can't get mesh group for parent region mesh
	EXPECT_FALSE(childGroup.getMeshGroup(mesh3d).isValid());

	// test remove/contains
	EXPECT_EQ(RESULT_OK, meshGroup.removeElement(childElement));
	EXPECT_FALSE(meshGroup.containsElement(childElement));

	// test for empty groups
	childGroup = FieldGroup();  // so not rediscovered by name
	EXPECT_FALSE((nonEmptyGroup = group.getFirstNonEmptySubregionFieldGroup()).isValid());
	meshGroup = MeshGroup();  // otherwise child group remains
	EXPECT_EQ(RESULT_OK, group.removeEmptySubgroups());
	EXPECT_FALSE((childGroup = group.getSubregionFieldGroup(childRegion)).isValid());
}

// test that FieldGroup lives as long as any MeshGroup made for it
TEST(ZincFieldGroup, lifespan_mesh_group)
{
	ZincTestSetupCpp zinc;

	Mesh mesh = zinc.fm.findMeshByDimension(3);
	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());
	EXPECT_EQ(RESULT_OK, group.setName("group"));
	cmzn_field_id field_id = group.getId();
	EXPECT_NE(nullptr, field_id);

	MeshGroup meshGroup = group.createMeshGroup(mesh);
	EXPECT_TRUE(meshGroup.isValid());
	// the following releases the handle but should not destroy group
	group = FieldGroup();
	EXPECT_EQ(nullptr, group.getId());

	group = meshGroup.getFieldGroup();
	EXPECT_EQ(field_id, group.getId());
}

TEST(ZincFieldGroup, add_remove_elements_conditional)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
        resourcePath("fieldmodule/two_cubes.exformat").c_str()));

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());

	int size1, size2, size3;
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_EQ(2, size3 = mesh3d.getSize());
	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	EXPECT_EQ(11, size2 = mesh2d.getSize());
	Mesh mesh1d = zinc.fm.findMeshByDimension(1);
	EXPECT_EQ(20, size1 = mesh1d.getSize());

	MeshGroup facesMeshGroup = group.createMeshGroup(mesh2d);
	EXPECT_TRUE(facesMeshGroup.isValid());
	EXPECT_EQ(0, facesMeshGroup.getSize());

	EXPECT_EQ(ERROR_ARGUMENT, facesMeshGroup.addElementsConditional(Field()));
	EXPECT_EQ(ERROR_ARGUMENT, facesMeshGroup.removeElementsConditional(Field()));

	MeshGroup linesMeshGroup = group.createMeshGroup(mesh1d);
	EXPECT_TRUE(linesMeshGroup.isValid());
	EXPECT_EQ(0, linesMeshGroup.getSize());

	FieldIsOnFace isOnFaceXi1_0 = zinc.fm.createFieldIsOnFace(Element::FACE_TYPE_XI1_0);
	EXPECT_TRUE(isOnFaceXi1_0.isValid());
	FieldIsOnFace isOnFaceXi1_1 = zinc.fm.createFieldIsOnFace(Element::FACE_TYPE_XI1_1);
	EXPECT_TRUE(isOnFaceXi1_1.isValid());

	const double oneValue = 1.0;
	FieldConstant trueField = zinc.fm.createFieldConstant(1, &oneValue);
	EXPECT_TRUE(trueField.isValid());

	Element face1 = mesh2d.findElementByIdentifier(1);
	Element face2 = mesh2d.findElementByIdentifier(2);
	Element face3 = mesh2d.findElementByIdentifier(3);

	EXPECT_EQ(RESULT_OK, facesMeshGroup.addElementsConditional(isOnFaceXi1_1));
	EXPECT_EQ(2, result = facesMeshGroup.getSize());
	EXPECT_TRUE(facesMeshGroup.containsElement(face2));
	EXPECT_TRUE(facesMeshGroup.containsElement(face3));

	EXPECT_EQ(0, linesMeshGroup.getSize());

	// copy into another mesh group to test conditional add/removal
	// optimisation for smaller groups
	FieldGroup otherGroup = zinc.fm.createFieldGroup();
	EXPECT_TRUE(otherGroup.isValid());
	MeshGroup otherFacesMeshGroup = otherGroup.createMeshGroup(mesh2d);
	EXPECT_TRUE(otherFacesMeshGroup.isValid());
	EXPECT_EQ(RESULT_OK, otherFacesMeshGroup.addElementsConditional(group));
	EXPECT_EQ(2, result = otherFacesMeshGroup.getSize());
	EXPECT_TRUE(otherFacesMeshGroup.containsElement(face2));
	EXPECT_TRUE(otherFacesMeshGroup.containsElement(face3));

	// remove faces conditionally with partially overlapping group
	// tests issue where remove conditional stopped at first element not in target group
	EXPECT_EQ(RESULT_OK, facesMeshGroup.removeElementsConditional(isOnFaceXi1_0));
	EXPECT_EQ(1, result = facesMeshGroup.getSize());
	EXPECT_TRUE(facesMeshGroup.containsElement(face3));

	// add all faces for conditional removal
	EXPECT_EQ(RESULT_OK, facesMeshGroup.addElementsConditional(trueField));
	EXPECT_EQ(11, result = facesMeshGroup.getSize());

	EXPECT_EQ(RESULT_OK, facesMeshGroup.removeElementsConditional(isOnFaceXi1_0));
	EXPECT_EQ(9, result = facesMeshGroup.getSize());
	EXPECT_FALSE(facesMeshGroup.containsElement(face1));
	EXPECT_FALSE(facesMeshGroup.containsElement(face2));

	// test using self as a conditional
	EXPECT_EQ(RESULT_OK, facesMeshGroup.removeElementsConditional(group));
	EXPECT_EQ(0, facesMeshGroup.getSize());

	EXPECT_EQ(RESULT_OK, facesMeshGroup.addElementsConditional(trueField));
	EXPECT_EQ(11, result = facesMeshGroup.getSize());

	EXPECT_EQ(RESULT_OK, facesMeshGroup.removeElementsConditional(otherGroup));
	EXPECT_EQ(9, result = facesMeshGroup.getSize());
	EXPECT_FALSE(facesMeshGroup.containsElement(face2));
	EXPECT_FALSE(facesMeshGroup.containsElement(face3));

	// check not an error to remove nodes already removed
	EXPECT_EQ(RESULT_OK, facesMeshGroup.removeAllElements());
	EXPECT_EQ(0, result = facesMeshGroup.getSize());
	EXPECT_EQ(RESULT_OK, facesMeshGroup.removeElementsConditional(otherGroup));
}

TEST(ZincFieldGroup, add_remove_nodes_conditional)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
        resourcePath("fieldmodule/two_cubes.exformat").c_str()));

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());

	int size;
	Nodeset nodeset = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_EQ(12, size = nodeset.getSize());

	NodesetGroup nodesetGroup = group.createNodesetGroup(nodeset);
	EXPECT_TRUE(nodesetGroup.isValid());
	EXPECT_EQ(0, nodesetGroup.getSize());

	EXPECT_EQ(ERROR_ARGUMENT, nodesetGroup.addNodesConditional(Field()));
	EXPECT_EQ(ERROR_ARGUMENT, nodesetGroup.removeNodesConditional(Field()));

	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	FieldComponent x = zinc.fm.createFieldComponent(coordinates, 1);
	EXPECT_TRUE(x.isValid());
	const double const5 = 5.0;
	FieldGreaterThan x_gt_5 = x > zinc.fm.createFieldConstant(1, &const5);
	EXPECT_TRUE(x_gt_5.isValid());
	const double const15 = 15.0;
	FieldLessThan x_lt_15 = x < zinc.fm.createFieldConstant(1, &const15);
	EXPECT_TRUE(x_lt_15.isValid());

	const double oneValue = 1.0;
	FieldConstant trueField = zinc.fm.createFieldConstant(1, &oneValue);
	EXPECT_TRUE(trueField.isValid());

	EXPECT_EQ(RESULT_OK, nodesetGroup.addNodesConditional(x_gt_5));
	EXPECT_EQ(8, result = nodesetGroup.getSize());
	for (int i = 1; i <= 12; ++i)
	{
		if ((i % 3) != 1)
		{
			EXPECT_TRUE(nodesetGroup.containsNode(nodeset.findNodeByIdentifier(i)));
		}
	}

	// copy into another mesh group to test conditional add/removal
	// optimisation for smaller groups
	FieldGroup otherGroup = zinc.fm.createFieldGroup();
	EXPECT_TRUE(otherGroup.isValid());
	NodesetGroup otherNodesetGroup = otherGroup.createNodesetGroup(nodeset);
	EXPECT_TRUE(otherNodesetGroup.isValid());
	EXPECT_EQ(RESULT_OK, otherNodesetGroup.addNodesConditional(group));
	EXPECT_EQ(8, result = otherNodesetGroup.getSize());
	for (int i = 1; i <= 12; ++i)
	{
		if ((i % 3) != 1)
		{
			EXPECT_TRUE(otherNodesetGroup.containsNode(nodeset.findNodeByIdentifier(i)));
		}
	}

	// remove nodes conditionally with partially overlapping group
	// tests issue where remove conditional stopped at first node not in target group
	EXPECT_EQ(RESULT_OK, nodesetGroup.removeNodesConditional(x_lt_15));
	EXPECT_EQ(4, result = nodesetGroup.getSize());
	for (int i = 3; i <= 12; i += 3)
	{
		EXPECT_TRUE(otherNodesetGroup.containsNode(nodeset.findNodeByIdentifier(i)));
	}

	// add all nodes for conditional removal
	EXPECT_EQ(RESULT_OK, nodesetGroup.addNodesConditional(trueField));
	EXPECT_EQ(12, result = nodesetGroup.getSize());

	EXPECT_EQ(RESULT_OK, nodesetGroup.removeNodesConditional(x_gt_5));
	EXPECT_EQ(4, result = nodesetGroup.getSize());
	for (int i = 1; i <= 10; i += 3)
	{
		EXPECT_TRUE(nodesetGroup.containsNode(nodeset.findNodeByIdentifier(i)));
	}

	// test using self as a conditional
	EXPECT_EQ(RESULT_OK, nodesetGroup.removeNodesConditional(group));
	EXPECT_EQ(0, nodesetGroup.getSize());

	EXPECT_EQ(RESULT_OK, nodesetGroup.addNodesConditional(trueField));
	EXPECT_EQ(12, result = nodesetGroup.getSize());

	EXPECT_EQ(RESULT_OK, nodesetGroup.removeNodesConditional(otherGroup));
	EXPECT_EQ(4, result = nodesetGroup.getSize());
	for (int i = 1; i <= 10; i += 3)
	{
		EXPECT_TRUE(nodesetGroup.containsNode(nodeset.findNodeByIdentifier(i)));
	}

	// check not an error to remove nodes already removed
	EXPECT_EQ(RESULT_OK, nodesetGroup.removeAllNodes());
	EXPECT_EQ(0, result = nodesetGroup.getSize());
	EXPECT_EQ(RESULT_OK, nodesetGroup.removeNodesConditional(otherGroup));
}

TEST(ZincFieldGroup, subelementHandlingMode)
{
	ZincTestSetupCpp zinc;
	int result;

	Region childRegion = zinc.root_region.createChild("child");
	EXPECT_TRUE(childRegion.isValid());

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());

	FieldGroup::SubelementHandlingMode mode;
	EXPECT_EQ(FieldGroup::SUBELEMENT_HANDLING_MODE_NONE, mode = group.getSubelementHandlingMode());
	EXPECT_EQ(ERROR_ARGUMENT, result = group.setSubelementHandlingMode(FieldGroup::SUBELEMENT_HANDLING_MODE_INVALID));
	EXPECT_EQ(RESULT_OK, result = group.setSubelementHandlingMode(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL));
	EXPECT_EQ(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL, mode = group.getSubelementHandlingMode());

	// check subelement handling mode is inherited by new subregion groups
	FieldGroup childGroup = group.createSubregionFieldGroup(childRegion);
	EXPECT_TRUE(childGroup.isValid());
	EXPECT_EQ(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL, mode = childGroup.getSubelementHandlingMode());

	EXPECT_EQ(RESULT_OK, result = group.setSubelementHandlingMode(FieldGroup::SUBELEMENT_HANDLING_MODE_NONE));
	EXPECT_EQ(FieldGroup::SUBELEMENT_HANDLING_MODE_NONE, mode = childGroup.getSubelementHandlingMode());

	// orphan child group, set new handling mode, rediscover child group and check mode
	EXPECT_EQ(RESULT_OK, result = group.clear());
	EXPECT_EQ(RESULT_OK, result = group.removeEmptySubgroups());
	EXPECT_EQ(RESULT_OK, result = group.setSubelementHandlingMode(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL));
	EXPECT_EQ(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL, mode = group.getSubelementHandlingMode());
	EXPECT_EQ(FieldGroup::SUBELEMENT_HANDLING_MODE_NONE, mode = childGroup.getSubelementHandlingMode());

	// subregion group can now be rediscovered by name using getSubregionFieldGroup()
	FieldGroup tempGroup = group.getSubregionFieldGroup(childRegion);
	EXPECT_TRUE(tempGroup.isValid());
	EXPECT_EQ(childGroup, tempGroup);
	EXPECT_EQ(FieldGroup::SUBELEMENT_HANDLING_MODE_NONE, mode = childGroup.getSubelementHandlingMode());
}

TEST(ZincFieldElementGroup, add_remove_with_subelement_handling)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(RESULT_OK, result = zinc.root_region.readFile(
        resourcePath("fieldmodule/two_cubes.exformat").c_str()));

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());
	EXPECT_EQ(RESULT_OK, result = group.setName("group"));
	FieldGroup::SubelementHandlingMode mode;
	EXPECT_EQ(RESULT_OK, result = group.setSubelementHandlingMode(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL));
	EXPECT_EQ(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL, mode = group.getSubelementHandlingMode());

	int size0, size1, size2, size3;
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_EQ(2, size3 = mesh3d.getSize());
	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	EXPECT_EQ(11, size2 = mesh2d.getSize());
	Mesh mesh1d = zinc.fm.findMeshByDimension(1);
	EXPECT_EQ(20, size1 = mesh1d.getSize());
	Nodeset nodeset = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_EQ(12, size0 = nodeset.getSize());

	Element element1 = mesh3d.findElementByIdentifier(1);
	EXPECT_TRUE(element1.isValid());
	Element element2 = mesh3d.findElementByIdentifier(2);
	EXPECT_TRUE(element2.isValid());

	MeshGroup elementsMeshGroup = group.createMeshGroup(mesh3d);
	EXPECT_TRUE(elementsMeshGroup.isValid());
	EXPECT_EQ(0, elementsMeshGroup.getSize());

	MeshGroup facesMeshGroup = group.createMeshGroup(mesh2d);
	EXPECT_TRUE(facesMeshGroup.isValid());

	MeshGroup linesMeshGroup = group.createMeshGroup(mesh1d);
	EXPECT_TRUE(linesMeshGroup.isValid());

	NodesetGroup nodesetGroup = group.createNodesetGroup(nodeset);
	EXPECT_TRUE(nodesetGroup.isValid());

	EXPECT_EQ(0, elementsMeshGroup.getSize());
	EXPECT_EQ(0, facesMeshGroup.getSize());
	EXPECT_EQ(0, linesMeshGroup.getSize());
	EXPECT_EQ(0, nodesetGroup.getSize());

	const double oneValue = 1.0;
	FieldConstant trueField = zinc.fm.createFieldConstant(1, &oneValue);
	EXPECT_TRUE(trueField.isValid());

	EXPECT_EQ(RESULT_OK, result = elementsMeshGroup.addElementsConditional(trueField));
	EXPECT_EQ(2, size3 = elementsMeshGroup.getSize());
	EXPECT_EQ(11, size2 = facesMeshGroup.getSize());
	EXPECT_EQ(20, size1 = linesMeshGroup.getSize());
	EXPECT_EQ(12, size0 = nodesetGroup.getSize());

	EXPECT_EQ(RESULT_OK, result = elementsMeshGroup.removeAllElements());
	EXPECT_EQ(0, result = elementsMeshGroup.getSize());
	EXPECT_EQ(0, result = facesMeshGroup.getSize());
	EXPECT_EQ(0, result = linesMeshGroup.getSize());
	EXPECT_EQ(0, result = nodesetGroup.getSize());

	EXPECT_EQ(RESULT_OK, result = elementsMeshGroup.addElement(element1));
	EXPECT_EQ(1, size3 = elementsMeshGroup.getSize());
	EXPECT_EQ(6, size2 = facesMeshGroup.getSize());
	EXPECT_EQ(12, size1 = linesMeshGroup.getSize());
	EXPECT_EQ(8, size0 = nodesetGroup.getSize());
	EXPECT_EQ(RESULT_OK, result = elementsMeshGroup.addElement(element2));
	EXPECT_EQ(2, size3 = elementsMeshGroup.getSize());
	EXPECT_EQ(11, size2 = facesMeshGroup.getSize());
	EXPECT_EQ(20, size1 = linesMeshGroup.getSize());
	EXPECT_EQ(12, size0 = nodesetGroup.getSize());

	EXPECT_EQ(RESULT_OK, result = elementsMeshGroup.removeElement(element1));
	EXPECT_EQ(1, size3 = elementsMeshGroup.getSize());
	EXPECT_EQ(6, size2 = facesMeshGroup.getSize());
	EXPECT_EQ(12, size1 = linesMeshGroup.getSize());
	EXPECT_EQ(8, size0 = nodesetGroup.getSize());
	EXPECT_EQ(RESULT_OK, result = elementsMeshGroup.removeElement(element2));
	EXPECT_EQ(0, result = elementsMeshGroup.getSize());
	EXPECT_EQ(0, result = facesMeshGroup.getSize());
	EXPECT_EQ(0, result = linesMeshGroup.getSize());
	EXPECT_EQ(0, result = nodesetGroup.getSize());

	FieldIsOnFace isOnFaceField = zinc.fm.createFieldIsOnFace(Element::FACE_TYPE_XI2_0);
	EXPECT_TRUE(isOnFaceField.isValid());

	EXPECT_EQ(RESULT_OK, result = facesMeshGroup.addElementsConditional(isOnFaceField));
	EXPECT_EQ(0, size3 = elementsMeshGroup.getSize());
	EXPECT_EQ(2, size2 = facesMeshGroup.getSize());
	EXPECT_EQ(7, size1 = linesMeshGroup.getSize());
	EXPECT_EQ(6, size0 = nodesetGroup.getSize());

	EXPECT_EQ(RESULT_OK, result = facesMeshGroup.addElementsConditional(trueField));
	EXPECT_EQ(0, size3 = elementsMeshGroup.getSize());
	EXPECT_EQ(11, size2 = facesMeshGroup.getSize());
	EXPECT_EQ(20, size1 = linesMeshGroup.getSize());
	EXPECT_EQ(12, size0 = nodesetGroup.getSize());

	EXPECT_EQ(RESULT_OK, result = facesMeshGroup.removeElementsConditional(!isOnFaceField));
	EXPECT_EQ(0, size3 = elementsMeshGroup.getSize());
	EXPECT_EQ(2, size2 = facesMeshGroup.getSize());
	EXPECT_EQ(7, size1 = linesMeshGroup.getSize());
	EXPECT_EQ(6, size0 = nodesetGroup.getSize());

	EXPECT_EQ(RESULT_OK, result = facesMeshGroup.removeElementsConditional(group));
	EXPECT_EQ(0, result = elementsMeshGroup.getSize());
	EXPECT_EQ(0, result = facesMeshGroup.getSize());
	EXPECT_EQ(0, result = linesMeshGroup.getSize());
	EXPECT_EQ(0, result = nodesetGroup.getSize());

	// test removal of lines appropriately leaves nodes behind
	Element line1 = mesh1d.findElementByIdentifier(1);
	EXPECT_TRUE(line1.isValid());
	Element line3 = mesh1d.findElementByIdentifier(3);
	EXPECT_TRUE(line3.isValid());
	Element line13 = mesh1d.findElementByIdentifier(13);
	EXPECT_TRUE(line13.isValid());
	Element line14 = mesh1d.findElementByIdentifier(14);
	EXPECT_TRUE(line14.isValid());

	EXPECT_EQ(RESULT_OK, result = linesMeshGroup.addElement(line1));
	EXPECT_EQ(RESULT_OK, result = linesMeshGroup.addElement(line3));
	EXPECT_EQ(RESULT_OK, result = linesMeshGroup.addElement(line13));
	EXPECT_EQ(RESULT_OK, result = linesMeshGroup.addElement(line14));
	EXPECT_EQ(0, size3 = elementsMeshGroup.getSize());
	EXPECT_EQ(0, size2 = facesMeshGroup.getSize());
	EXPECT_EQ(4, size1 = linesMeshGroup.getSize());
	EXPECT_EQ(4, size0 = nodesetGroup.getSize());
	EXPECT_EQ(RESULT_OK, result = linesMeshGroup.removeElement(line1));
	EXPECT_EQ(3, size1 = linesMeshGroup.getSize());
	EXPECT_EQ(4, size0 = nodesetGroup.getSize());

	// fill group, ready to test clear()
	EXPECT_EQ(RESULT_OK, result = elementsMeshGroup.addElementsConditional(trueField));
	EXPECT_EQ(2, size3 = elementsMeshGroup.getSize());
	EXPECT_EQ(11, size2 = facesMeshGroup.getSize());
	EXPECT_EQ(20, size1 = linesMeshGroup.getSize());
	EXPECT_EQ(12, size0 = nodesetGroup.getSize());

	// remove handles to subobject group fields/domains so they can be cleaned up
	char *name;
	name = elementsMeshGroup.getName();
	EXPECT_STREQ("group.mesh3d", name);
	cmzn_deallocate(name);
	name = nodesetGroup.getName();
	EXPECT_STREQ("group.nodes", name);
	cmzn_deallocate(name);
	elementsMeshGroup = MeshGroup();
	facesMeshGroup = MeshGroup();
	linesMeshGroup = MeshGroup();
	nodesetGroup = NodesetGroup();

	// check clear empties and orphans subobject groups
	Mesh tempMesh;
	Nodeset tempNodeset;
	EXPECT_EQ(OK, result = group.clear());

	EXPECT_FALSE(group.getMeshGroup(mesh3d).isValid());
	EXPECT_FALSE(zinc.fm.findMeshByName("group.mesh3d").isValid());

	EXPECT_FALSE(group.getMeshGroup(mesh2d).isValid());
	EXPECT_FALSE(zinc.fm.findMeshByName("group.mesh2d").isValid());

	EXPECT_FALSE(group.getMeshGroup(mesh1d).isValid());
	EXPECT_FALSE(zinc.fm.findMeshByName("group.mesh1d").isValid());

	EXPECT_FALSE(group.getNodesetGroup(nodeset).isValid());
	EXPECT_FALSE(zinc.fm.findNodesetByName("group.nodes").isValid());
}

TEST(ZincMesh, castGroup)
{
	ZincTestSetupCpp zinc;
	int result;

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());
	EXPECT_EQ(CMZN_OK, result = group.setName("group"));

	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_TRUE(mesh3d.isValid());
	MeshGroup noMeshGroup = mesh3d.castGroup();
	EXPECT_FALSE(noMeshGroup.isValid());

	MeshGroup expectedMeshGroup = group.createMeshGroup(mesh3d);
	EXPECT_TRUE(expectedMeshGroup.isValid());

	Mesh findMesh = zinc.fm.findMeshByName("group.mesh3d");
	EXPECT_TRUE(findMesh.isValid());

	MeshGroup findMeshGroup = findMesh.castGroup();
	EXPECT_TRUE(findMesh.isValid());

	EXPECT_EQ(expectedMeshGroup, findMeshGroup);
}

TEST(ZincNodeset, castGroup)
{
	ZincTestSetupCpp zinc;
	int result;

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());
	EXPECT_EQ(CMZN_OK, result = group.setName("group"));

	Nodeset nodeset = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodeset.isValid());
	NodesetGroup noNodesetGroup = nodeset.castGroup();
	EXPECT_FALSE(noNodesetGroup.isValid());

	NodesetGroup expectedNodesetGroup = group.createNodesetGroup(nodeset);
	EXPECT_TRUE(expectedNodesetGroup.isValid());

	Nodeset findNodeset = zinc.fm.findNodesetByName("group.nodes");
	EXPECT_TRUE(findNodeset.isValid());

	NodesetGroup findNodesetGroup = findNodeset.castGroup();
	EXPECT_TRUE(findNodeset.isValid());

	EXPECT_EQ(expectedNodesetGroup, findNodesetGroup);
}

namespace {

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

} // anonymous namespace

// Test that emptying subobject or subregion groups then removing empty
// subgroups - while fieldmodule changes are stopped - correctly propagates
// a change. There was a defect where this was reported as no change.
TEST(ZincFieldGroup, removeEmptySubgroupsPropagatesChanges)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
        resourcePath("fieldmodule/two_cubes.exformat").c_str()));

	int size0, size3;
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_EQ(2, size3 = mesh3d.getSize());
	Nodeset nodeset = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_EQ(12, size0 = nodeset.getSize());

	Element element1 = mesh3d.findElementByIdentifier(1);
	EXPECT_TRUE(element1.isValid());
	Node node1 = nodeset.findNodeByIdentifier(1);
	EXPECT_TRUE(node1.isValid());

	// set up notification of scene selection group changes
	
	Selectionnotifier selectionnotifier = zinc.scene.createSelectionnotifier();
	EXPECT_TRUE(selectionnotifier.isValid());

	SelectioncallbackRecordChange callback;
	EXPECT_EQ(OK, result = selectionnotifier.setCallback(callback));

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());
	EXPECT_EQ(OK, result = group.setName("group"));

	MeshGroup meshGroup = group.createMeshGroup(mesh3d);
	EXPECT_TRUE(meshGroup.isValid());
	EXPECT_EQ(0, meshGroup.getSize());
	EXPECT_EQ(OK, meshGroup.addElement(element1));

	// test that setting a non-empty selection group notifies
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_NONE, result = callback.getChangeSummary());
	EXPECT_EQ(OK, zinc.scene.setSelectionField(group));
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_ADD, result = callback.getChangeSummary());

	// test that removing an element correctly propagates to the parent group
	callback.clear();
	EXPECT_EQ(OK, meshGroup.removeElement(element1));
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_REMOVE, result = callback.getChangeSummary());

	// test that re-adding an element correctly propagates to the parent group
	callback.clear();
	EXPECT_EQ(OK, meshGroup.addElement(element1));
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_ADD, result = callback.getChangeSummary());

	// test that removing an element and flushing empty subgroups while
	// caching changes correctly propagates to the parent group
	callback.clear();
	zinc.fm.beginChange();
	EXPECT_EQ(OK, meshGroup.removeElement(element1));
	EXPECT_EQ(OK, group.removeEmptySubgroups());
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_NONE, result = callback.getChangeSummary());
	zinc.fm.endChange();
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_REMOVE, result = callback.getChangeSummary());
	// show mesh group is not removed while access held
	EXPECT_TRUE(group.getMeshGroup(mesh3d).isValid());
	meshGroup = MeshGroup();
	EXPECT_EQ(OK, group.removeEmptySubgroups());
	EXPECT_FALSE(group.getMeshGroup(mesh3d).isValid());

	NodesetGroup nodesetGroup = group.createNodesetGroup(nodeset);
	EXPECT_TRUE(nodesetGroup.isValid());
	EXPECT_EQ(0, nodesetGroup.getSize());

	// test that adding a node correctly propagates to the parent group
	callback.clear();
	EXPECT_EQ(OK, nodesetGroup.addNode(node1));
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_ADD, result = callback.getChangeSummary());

	// test that removing node correctly propagates to the parent group
	callback.clear();
	EXPECT_EQ(OK, nodesetGroup.removeNode(node1));
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_REMOVE, result = callback.getChangeSummary());
	EXPECT_EQ(OK, nodesetGroup.addNode(node1)); // re-add node

	// test that removing a node and flushing empty subgroups while
	// caching changes correctly propagates to the parent group
	callback.clear();
	zinc.fm.beginChange();
	EXPECT_EQ(OK, nodesetGroup.removeNode(node1));
	EXPECT_EQ(OK, group.removeEmptySubgroups());
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_NONE, result = callback.getChangeSummary());
	zinc.fm.endChange();
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_REMOVE, result = callback.getChangeSummary());
	// show nodeset group is not removed while access held
	EXPECT_TRUE(group.getNodesetGroup(nodeset).isValid());
	nodesetGroup = NodesetGroup();
	EXPECT_EQ(OK, group.removeEmptySubgroups());
	EXPECT_FALSE(group.getNodesetGroup(nodeset).isValid());

	// test propagation of change from adding a child region to group
	Region childRegion = zinc.root_region.createChild("child");
	EXPECT_TRUE(childRegion.isValid());
	FieldGroup childGroup = group.getSubregionFieldGroup(childRegion);
	EXPECT_FALSE(childGroup.isValid());
	callback.clear();
	EXPECT_EQ(OK, result = group.addRegion(childRegion));
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_ADD, result = callback.getChangeSummary());
	childGroup = group.getSubregionFieldGroup(childRegion);
	EXPECT_TRUE(childGroup.isValid());

	// test propagation of change from removing child region
	callback.clear();
	EXPECT_EQ(OK, result = group.removeRegion(childRegion));
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_REMOVE, result = callback.getChangeSummary());
	EXPECT_EQ(OK, result = group.addRegion(childRegion));

	// test that removing child region and flushing empty subgroups while
	// caching changes correctly propagates to the parent group
	callback.clear();
	zinc.fm.beginChange();
	childGroup = FieldGroup();  // so not rediscovered by name
	EXPECT_EQ(OK, result = group.removeRegion(childRegion));
	EXPECT_EQ(OK, group.removeEmptySubgroups());
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_NONE, result = callback.getChangeSummary());
	zinc.fm.endChange();
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_REMOVE, result = callback.getChangeSummary());

	childGroup = group.getSubregionFieldGroup(childRegion);
	EXPECT_FALSE(childGroup.isValid());

	EXPECT_EQ(OK, result = group.addRegion(childRegion));
	childGroup = group.getSubregionFieldGroup(childRegion);
	EXPECT_TRUE(childGroup.isValid());

	// test that removing child region local group and flushing empty subgroups while
	// caching hierarchical changes correctly propagates to the parent group
	callback.clear();
	zinc.root_region.beginHierarchicalChange();
	EXPECT_EQ(OK, result = childGroup.clearLocal());
	EXPECT_EQ(OK, group.removeEmptySubgroups());
	childGroup = FieldGroup();
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_NONE, result = callback.getChangeSummary());
	zinc.root_region.endHierarchicalChange();
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_REMOVE, result = callback.getChangeSummary());
}

// Issue 3852: Check changes to group propagate to fields depending on mesh/nodeset
TEST(ZincFieldGroup, issue_3852_dependency_on_mesh_or_nodeset_group)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
        resourcePath("fieldmodule/cube.exformat").c_str()));

	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());

	Mesh mesh = zinc.fm.findMeshByDimension(2);
	EXPECT_TRUE(mesh.isValid());
	Nodeset nodeset = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodeset.isValid());

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());
	EXPECT_EQ(OK, result = group.setName("group"));
	EXPECT_EQ(OK, result = group.setSubelementHandlingMode(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL));

	MeshGroup meshGroup = group.createMeshGroup(mesh);
	EXPECT_TRUE(meshGroup.isValid());

	NodesetGroup nodesetGroup = group.createNodesetGroup(nodeset);
	EXPECT_TRUE(nodesetGroup.isValid());

	Element element1 = mesh.findElementByIdentifier(1);
	EXPECT_TRUE(element1.isValid());
	EXPECT_EQ(OK, result = meshGroup.addElement(element1));

	const double valueOne = 1.0;
	Field one = zinc.fm.createFieldConstant(1, &valueOne);
	FieldMeshIntegral groupArea = zinc.fm.createFieldMeshIntegral(one, coordinates, meshGroup);
	EXPECT_TRUE(groupArea.isValid());
	FieldNodesetSum groupNodes = zinc.fm.createFieldNodesetSum(one, nodesetGroup);
	EXPECT_TRUE(groupNodes.isValid());
	FieldFindMeshLocation findMeshLocation = zinc.fm.createFieldFindMeshLocation(coordinates, coordinates, meshGroup);
	EXPECT_TRUE(findMeshLocation.isValid());
	EXPECT_EQ(OK, result = findMeshLocation.setSearchMode(FieldFindMeshLocation::SEARCH_MODE_NEAREST));

	Fieldcache cache = zinc.fm.createFieldcache();
	Node node6 = nodeset.findNodeByIdentifier(6);
	EXPECT_TRUE(node6.isValid());
	EXPECT_EQ(OK, result = cache.setNode(node6));
	double area, count, findXi[2];
	Element findElement;

	EXPECT_EQ(OK, result = groupArea.evaluateReal(cache, 1, &area));
	EXPECT_DOUBLE_EQ(1.0, area);
	EXPECT_EQ(OK, result = groupNodes.evaluateReal(cache, 1, &count));
	EXPECT_DOUBLE_EQ(4.0, count);
	findElement = findMeshLocation.evaluateMeshLocation(cache, 2, findXi);
	EXPECT_EQ(element1, findElement);
	EXPECT_DOUBLE_EQ(0.0, findXi[0]);
	EXPECT_DOUBLE_EQ(1.0, findXi[1]);

	Element element3 = mesh.findElementByIdentifier(3);
	EXPECT_TRUE(element3.isValid());
	EXPECT_EQ(OK, result = meshGroup.addElement(element3));

	EXPECT_EQ(OK, result = groupArea.evaluateReal(cache, 1, &area));
	EXPECT_DOUBLE_EQ(2.0, area);
	EXPECT_EQ(OK, result = groupNodes.evaluateReal(cache, 1, &count));
	EXPECT_DOUBLE_EQ(6.0, count);
	findElement = findMeshLocation.evaluateMeshLocation(cache, 2, findXi);
	EXPECT_EQ(element3, findElement);
	EXPECT_DOUBLE_EQ(1.0, findXi[0]);
	EXPECT_DOUBLE_EQ(1.0, findXi[1]);
};

TEST(ZincFieldElementGroup, ElementiteratorInvalidation)
{
	ZincTestSetupCpp zinc;

	Region childRegion = zinc.root_region.createChild("temp");
	Fieldmodule fm = childRegion.getFieldmodule();

	Mesh mesh = fm.findMeshByDimension(3);
	Elementtemplate elementTemplate = mesh.createElementtemplate();
	EXPECT_TRUE(elementTemplate.isValid());
	EXPECT_EQ(OK, elementTemplate.setElementShapeType(Element::SHAPE_TYPE_CUBE));

	FieldGroup group = fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());
	MeshGroup meshGroup = group.createMeshGroup(mesh);
	EXPECT_TRUE(meshGroup.isValid());

	Element element[32];
	for (int e = 0; e < 31; ++e)
	{
		element[e] = meshGroup.createElement(e + 1, elementTemplate);
		EXPECT_TRUE(element[e].isValid());
	}
	EXPECT_EQ(31, meshGroup.getSize());

	Elementiterator iter;
	Element el;

	// test that creating a new element safely invalidates iterator
	iter = meshGroup.createElementiterator();
	EXPECT_TRUE(iter.isValid());
	el = iter.next();
	EXPECT_EQ(element[0], el);
	el = iter.next();
	EXPECT_EQ(element[1], el);
	element[31] = meshGroup.createElement(32, elementTemplate);
	EXPECT_TRUE(element[31].isValid());
	EXPECT_EQ(32, meshGroup.getSize());
	el = iter.next();
	EXPECT_FALSE(el.isValid());

	// test that removing an element safely invalidates iterator
	elementTemplate = Elementtemplate();
	fm = Fieldmodule();
	iter = meshGroup.createElementiterator();
	EXPECT_TRUE(iter.isValid());
	el = iter.next();
	EXPECT_EQ(element[0], el);
	el = iter.next();
	EXPECT_EQ(element[1], el);
	EXPECT_EQ(OK, meshGroup.removeElement(element[3]));
	EXPECT_EQ(31, meshGroup.getSize());
	el = iter.next();
	EXPECT_FALSE(el.isValid());

	// test that destroying an element (even if not in group) safely invalidates iterator
	elementTemplate = Elementtemplate();
	fm = Fieldmodule();
	iter = meshGroup.createElementiterator();
	EXPECT_TRUE(iter.isValid());
	el = iter.next();
	EXPECT_EQ(element[0], el);
	el = iter.next();
	EXPECT_EQ(element[1], el);
	EXPECT_EQ(OK, mesh.destroyElement(element[5]));
	EXPECT_EQ(31, mesh.getSize());
	EXPECT_EQ(30, meshGroup.getSize());
	Mesh tmpMesh = element[5].getMesh();
	EXPECT_FALSE(tmpMesh.isValid());
	el = iter.next();
	EXPECT_FALSE(el.isValid());

	// test that renumbering an element safely invalidates iterator
	iter = meshGroup.createElementiterator();
	EXPECT_TRUE(iter.isValid());
	el = iter.next();
	EXPECT_EQ(element[0], el);
	el = iter.next();
	EXPECT_EQ(element[1], el);
	EXPECT_EQ(OK, element[31].setIdentifier(55));
	el = iter.next();
	EXPECT_FALSE(el.isValid());

	// test that destroying child region safely invalidates iterator
	iter = meshGroup.createElementiterator();
	EXPECT_TRUE(iter.isValid());
	el = iter.next();
	EXPECT_EQ(element[0], el);
	el = iter.next();
	EXPECT_EQ(element[1], el);
	EXPECT_EQ(30, meshGroup.getSize());
	EXPECT_EQ(OK, zinc.root_region.removeChild(childRegion));
	childRegion = Region(); // clear last handle so child region will be destroyed
	meshGroup = MeshGroup(); // clear handle
	group = FieldGroup(); // clear handle
	mesh = Mesh(); // clear handle
	el = iter.next();
	EXPECT_FALSE(el.isValid());
	tmpMesh = element[15].getMesh();
	EXPECT_FALSE(tmpMesh.isValid());
}

TEST(ZincFieldNodeGroup, NodeiteratorInvalidation)
{
	ZincTestSetupCpp zinc;

	Region childRegion = zinc.root_region.createChild("temp");
	Fieldmodule fm = childRegion.getFieldmodule();

	Nodeset nodeset = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	Nodetemplate nodeTemplate = nodeset.createNodetemplate();
	EXPECT_TRUE(nodeTemplate.isValid());

	FieldGroup group = fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());
	NodesetGroup nodesetGroup = group.createNodesetGroup(nodeset);
	EXPECT_TRUE(nodesetGroup.isValid());

	Node node[32];
	for (int n = 0; n < 31; ++n)
	{
		node[n] = nodesetGroup.createNode(n + 1, nodeTemplate);
		EXPECT_TRUE(node[n].isValid());
	}
	EXPECT_EQ(31, nodesetGroup.getSize());

	Nodeiterator iter;
	Node nd;

	// test that creating a new node safely invalidates iterator
	iter = nodesetGroup.createNodeiterator();
	EXPECT_TRUE(iter.isValid());
	nd = iter.next();
	EXPECT_EQ(node[0], nd);
	nd = iter.next();
	EXPECT_EQ(node[1], nd);
	node[31] = nodesetGroup.createNode(32, nodeTemplate);
	EXPECT_TRUE(node[31].isValid());
	EXPECT_EQ(32, nodesetGroup.getSize());
	nd = iter.next();
	EXPECT_FALSE(nd.isValid());

	// test that removing a node safely invalidates iterator
	nodeTemplate = Nodetemplate();
	fm = Fieldmodule();
	iter = nodesetGroup.createNodeiterator();
	EXPECT_TRUE(iter.isValid());
	nd = iter.next();
	EXPECT_EQ(node[0], nd);
	nd = iter.next();
	EXPECT_EQ(node[1], nd);
	EXPECT_EQ(OK, nodesetGroup.removeNode(node[3]));
	EXPECT_EQ(31, nodesetGroup.getSize());
	nd = iter.next();
	EXPECT_FALSE(nd.isValid());

	// test that destroying a node (even if not in group) safely invalidates iterator
	nodeTemplate = Nodetemplate();
	fm = Fieldmodule();
	iter = nodesetGroup.createNodeiterator();
	EXPECT_TRUE(iter.isValid());
	nd = iter.next();
	EXPECT_EQ(node[0], nd);
	nd = iter.next();
	EXPECT_EQ(node[1], nd);
	EXPECT_EQ(OK, nodeset.destroyNode(node[5]));
	EXPECT_EQ(31, nodeset.getSize());
	EXPECT_EQ(30, nodesetGroup.getSize());
	Nodeset tmpNodeset = node[5].getNodeset();
	EXPECT_FALSE(tmpNodeset.isValid());
	nd = iter.next();
	EXPECT_FALSE(nd.isValid());

	// test that renumbering a node safely invalidates iterator
	iter = nodesetGroup.createNodeiterator();
	EXPECT_TRUE(iter.isValid());
	nd = iter.next();
	EXPECT_EQ(node[0], nd);
	nd = iter.next();
	EXPECT_EQ(node[1], nd);
	EXPECT_EQ(OK, node[31].setIdentifier(55));
	nd = iter.next();
	EXPECT_FALSE(nd.isValid());

	// test that destroying child region safely invalidates iterator
	iter = nodesetGroup.createNodeiterator();
	EXPECT_TRUE(iter.isValid());
	nd = iter.next();
	EXPECT_EQ(node[0], nd);
	nd = iter.next();
	EXPECT_EQ(node[1], nd);
	EXPECT_EQ(OK, zinc.root_region.removeChild(childRegion));
	childRegion = Region(); // clear last handle so child region will be destroyed
	nodesetGroup = NodesetGroup(); // clear handle
	group = FieldGroup(); // clear handle
	nodeset = Nodeset(); // clear handle
	nd = iter.next();
	EXPECT_FALSE(nd.isValid());
	tmpNodeset = node[15].getNodeset();
	EXPECT_FALSE(tmpNodeset.isValid());
}

// Test get/create subobject groups in subregions.
TEST(ZincFieldGroup, getCreateSubregionSubobjectGroups)
{
	ZincTestSetupCpp zinc;
	char* name;
	Field tmpField;

	Region child = zinc.root_region.createChild("child");
	EXPECT_TRUE(child.isValid());

	FieldGroup bob = zinc.fm.createFieldGroup();
	EXPECT_TRUE(bob.isValid());
	EXPECT_EQ(RESULT_OK, bob.setName("bob"));
	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	MeshGroup bobMesh2d = bob.getMeshGroup(mesh2d);
	EXPECT_FALSE(bobMesh2d.isValid());
	bobMesh2d = bob.createMeshGroup(mesh2d);
	EXPECT_TRUE(bobMesh2d.isValid());
	name = bobMesh2d.getName();
	EXPECT_STREQ("bob.mesh2d", name);
	cmzn_deallocate(name);
	Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	NodesetGroup bobNodes = bob.getNodesetGroup(nodes);
	EXPECT_FALSE(bobNodes.isValid());
	bobNodes = bob.createNodesetGroup(nodes);
	EXPECT_TRUE(bobNodes.isValid());
	name = bobNodes.getName();
	EXPECT_STREQ("bob.nodes", name);
	cmzn_deallocate(name);

	FieldGroup childBob = bob.getSubregionFieldGroup(child);
	EXPECT_FALSE(childBob.isValid());
	childBob = bob.createSubregionFieldGroup(child);
	EXPECT_TRUE(childBob.isValid());
	name = childBob.getName();
	EXPECT_STREQ("bob", name);
	cmzn_deallocate(name);
	Fieldmodule childFm = child.getFieldmodule();
	Mesh childMesh2d = childFm.findMeshByDimension(2);
	MeshGroup childBobMesh2d = bob.getMeshGroup(childMesh2d);
	EXPECT_FALSE(childBobMesh2d.isValid());
	childBobMesh2d = bob.createMeshGroup(childMesh2d);
	EXPECT_TRUE(childBobMesh2d.isValid());
	name = childBobMesh2d.getName();
	EXPECT_STREQ("bob.mesh2d", name);
	cmzn_deallocate(name);
	Nodeset childNodes = childFm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	NodesetGroup childBobNodes = bob.getNodesetGroup(childNodes);
	EXPECT_FALSE(childBobNodes.isValid());
	childBobNodes = bob.createNodesetGroup(childNodes);
	EXPECT_TRUE(childBobNodes.isValid());
	name = childBobNodes.getName();
	EXPECT_STREQ("bob.nodes", name);
	cmzn_deallocate(name);

	// orphan subobject groups in child region and try to get them back by name
	childBob = FieldGroup();
	bob.clear();
	bob.removeEmptySubgroups();
	tmpField = Field();
	MeshGroup childBobMesh2dGet = bob.getMeshGroup(childMesh2d);
	EXPECT_TRUE(childBobMesh2dGet.isValid());
	EXPECT_EQ(childBobMesh2d, childBobMesh2dGet);
	name = childBobMesh2dGet.getName();
	EXPECT_STREQ("bob.mesh2d", name);
	cmzn_deallocate(name);
	childBobMesh2dGet = MeshGroup();
	NodesetGroup childBobNodesGet = bob.getNodesetGroup(childNodes);
	EXPECT_TRUE(childBobNodesGet.isValid());
	EXPECT_EQ(childBobNodes, childBobNodesGet);
	name = childBobNodesGet.getName();
	EXPECT_STREQ("bob.nodes", name);
	cmzn_deallocate(name);
	childBobNodesGet = NodesetGroup();
	tmpField = childFm.findFieldByName("bob");
	EXPECT_TRUE(tmpField.isValid());
	tmpField = Field();

	// orphan subobject groups in child region and test can't create them
	childBob = FieldGroup();
	bob.clear();
	bob.removeEmptySubgroups();
	// child group "bob" is orphaned but stays around while handles to its mesh and nodeset groups exist
	childBob = bob.getSubregionFieldGroup(child);
	EXPECT_TRUE(childBob.isValid());
	// hence can't create its mesh and nodeset groups
	EXPECT_FALSE(bob.createMeshGroup(childMesh2d).isValid());
	EXPECT_FALSE(bob.createNodesetGroup(childNodes).isValid());

	// test can't get or create a subregion group if existing field of wrong type is using expected name
	FieldGroup fred = zinc.fm.createFieldGroup();
	EXPECT_TRUE(fred.isValid());
	EXPECT_EQ(RESULT_OK, fred.setName("fred"));
	const double one = 1.0;
	FieldConstant childFredMesh2dConstant = childFm.createFieldConstant(1, &one);
	EXPECT_TRUE(childFredMesh2dConstant.isValid());
	EXPECT_EQ(RESULT_OK, childFredMesh2dConstant.setName("fred"));
	EXPECT_FALSE(fred.getSubregionFieldGroup(child).isValid());
	EXPECT_FALSE(fred.createSubregionFieldGroup(child).isValid());
	EXPECT_FALSE(fred.getOrCreateSubregionFieldGroup(child).isValid());
}

TEST(ZincFieldGroup, removeEmptySubobjectGroups)
{
	ZincTestSetupCpp zinc;

	EXPECT_EQ(OK, zinc.root_region.readFile(
		resourcePath("fieldmodule/cube.exformat").c_str()));

	FieldGroup bob = zinc.fm.createFieldGroup();
	EXPECT_TRUE(bob.isValid());
	EXPECT_EQ(RESULT_OK, bob.setName("bob"));
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	MeshGroup bobMesh3d = bob.createMeshGroup(mesh3d);
	EXPECT_TRUE(bobMesh3d.isValid());
	EXPECT_EQ(RESULT_OK, bobMesh3d.addElement(mesh3d.findElementByIdentifier(1)));
	EXPECT_EQ(1, bobMesh3d.getSize());
	bobMesh3d = MeshGroup();  // non-zero size without an external handle
	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	MeshGroup bobMesh2d = bob.createMeshGroup(mesh2d);
	EXPECT_TRUE(bobMesh2d.isValid());
	Mesh mesh1d = zinc.fm.findMeshByDimension(1);
	MeshGroup bobMesh1d = bob.createMeshGroup(mesh1d);
	EXPECT_TRUE(bobMesh1d.isValid());
	Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	NodesetGroup bobNodes = bob.getNodesetGroup(nodes);
	EXPECT_FALSE(bobNodes.isValid());
	bobNodes = bob.createNodesetGroup(nodes);
	EXPECT_TRUE(bobNodes.isValid());

	// make empty child region group and empty mesh group to show not affected
	Region child = zinc.root_region.createChild("child");
	EXPECT_TRUE(child.isValid());
	FieldGroup childBob = bob.createSubregionFieldGroup(child);
	EXPECT_TRUE(childBob.isValid());
	Fieldmodule childFm = child.getFieldmodule();
	Mesh childMesh2d = childFm.findMeshByDimension(2);
	MeshGroup childBobMesh2d = bob.createMeshGroup(childMesh2d);
	EXPECT_TRUE(childBobMesh2d.isValid());
	childBobMesh2d = MeshGroup();

	EXPECT_EQ(RESULT_OK, bob.removeEmptySubobjectGroups());
	// nothing should have been removed either because not empty or handle held
	EXPECT_TRUE(bob.getMeshGroup(mesh3d).isValid());
	EXPECT_EQ(bobMesh2d, bob.getMeshGroup(mesh2d));
	EXPECT_EQ(bobMesh1d, bob.getMeshGroup(mesh1d));
	EXPECT_EQ(bobNodes, bob.getNodesetGroup(nodes));
	EXPECT_EQ(childBob, bob.getSubregionFieldGroup(child));
	EXPECT_TRUE(bob.getMeshGroup(childMesh2d).isValid());

	bobMesh2d = MeshGroup();
	bobMesh1d = MeshGroup();
	EXPECT_EQ(RESULT_OK, bobNodes.addNode(nodes.findNodeByIdentifier(1)));
	EXPECT_EQ(1, bobNodes.getSize());
	bobNodes = NodesetGroup();
	EXPECT_EQ(RESULT_OK, bob.removeEmptySubobjectGroups());
	// non-empty group and subregion groups are not affected
	EXPECT_TRUE(bob.getMeshGroup(mesh3d).isValid());
	EXPECT_FALSE(bob.getMeshGroup(mesh2d).isValid());
	EXPECT_FALSE(bob.getMeshGroup(mesh1d).isValid());
	EXPECT_TRUE(bob.getNodesetGroup(nodes).isValid());
	EXPECT_EQ(childBob, bob.getSubregionFieldGroup(child));
	EXPECT_TRUE(bob.getMeshGroup(childMesh2d).isValid());

	bob.getMeshGroup(mesh3d).removeAllElements();
	bob.getNodesetGroup(nodes).removeAllNodes();
	EXPECT_EQ(RESULT_OK, bob.removeEmptySubobjectGroups());
	EXPECT_FALSE(bob.getMeshGroup(mesh3d).isValid());
	EXPECT_FALSE(bob.getNodesetGroup(nodes).isValid());
}

TEST(ZincFieldGroup, removeSubregionFieldGroup)
{
	ZincTestSetupCpp zinc;

	EXPECT_EQ(OK, zinc.root_region.readFile(
		resourcePath("fieldmodule/cube.exformat").c_str()));

	FieldGroup bob = zinc.fm.createFieldGroup();
	EXPECT_TRUE(bob.isValid());
	EXPECT_EQ(RESULT_OK, bob.setName("bob"));

	Region child = zinc.root_region.createChild("child");
	EXPECT_TRUE(child.isValid());
	FieldGroup childBob = bob.createSubregionFieldGroup(child);
	EXPECT_TRUE(childBob.isValid());

	Region sibling = zinc.root_region.createChild("sibling");
	EXPECT_TRUE(sibling.isValid());

	Region grandchild = child.createChild("grandchild");
	EXPECT_TRUE(grandchild.isValid());
	FieldGroup grandchildBob = bob.createSubregionFieldGroup(grandchild);
	EXPECT_TRUE(grandchildBob.isValid());
	char* name = grandchildBob.getName();
	EXPECT_STREQ("bob", name);
	cmzn_deallocate(name);

	EXPECT_EQ(RESULT_ERROR_ARGUMENT, bob.removeSubregionFieldGroup(Region()));
	EXPECT_EQ(RESULT_ERROR_NOT_FOUND, bob.removeSubregionFieldGroup(sibling));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, childBob.removeSubregionFieldGroup(child));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, childBob.removeSubregionFieldGroup(sibling));

	EXPECT_EQ(RESULT_OK, bob.removeSubregionFieldGroup(grandchild));
	// Check can still find grandchildBob by name
	EXPECT_EQ(grandchildBob, grandchild.getFieldmodule().findFieldByName("bob"));
	grandchildBob = FieldGroup();
	EXPECT_FALSE(grandchild.getFieldmodule().findFieldByName("bob").isValid());
	EXPECT_EQ(RESULT_ERROR_NOT_FOUND, childBob.removeSubregionFieldGroup(grandchild));

	childBob = FieldGroup();
	EXPECT_TRUE(child.getFieldmodule().findFieldByName("bob").isValid());  // since held by parent bob
	EXPECT_EQ(RESULT_OK, bob.removeSubregionFieldGroup(child));
	EXPECT_FALSE(child.getFieldmodule().findFieldByName("bob").isValid());
	EXPECT_EQ(RESULT_ERROR_NOT_FOUND, bob.removeSubregionFieldGroup(child));
}

TEST(ZincFieldGroup, tetrahedralmesh_addAdjacentElements)
{
	ZincTestSetupCpp zinc;

	EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(resourcePath("fieldio/tetmesh.ex2").c_str()));
	EXPECT_EQ(RESULT_OK, zinc.fm.defineAllFaces());
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_EQ(102, mesh3d.getSize());
	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	EXPECT_EQ(232, mesh2d.getSize());
	Mesh mesh1d = zinc.fm.findMeshByDimension(1);
	EXPECT_EQ(167, mesh1d.getSize());
	Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_EQ(38, nodes.getSize());

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());
	EXPECT_EQ(RESULT_OK, group.setName("original"));
	EXPECT_EQ(RESULT_OK, group.setSubelementHandlingMode(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL));
	EXPECT_EQ(RESULT_OK, group.setManaged(true));
	Element element93 = mesh3d.findElementByIdentifier(93);
	MeshGroup meshGroup3d = group.createMeshGroup(mesh3d);
	EXPECT_TRUE(meshGroup3d.isValid());
	EXPECT_EQ(RESULT_OK, meshGroup3d.addElement(element93));
	EXPECT_EQ(1, meshGroup3d.getSize());
	MeshGroup meshGroup2d = group.getMeshGroup(mesh2d);
	EXPECT_TRUE(meshGroup2d.isValid());
	EXPECT_EQ(4, meshGroup2d.getSize());
	MeshGroup meshGroup1d = group.getMeshGroup(mesh1d);
	EXPECT_TRUE(meshGroup1d.isValid());
	EXPECT_EQ(6, meshGroup1d.getSize());

	// test some invalid arguments
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, MeshGroup().addAdjacentElements(0));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, meshGroup3d.addAdjacentElements(3));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, meshGroup3d.addAdjacentElements(-4));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, meshGroup2d.addAdjacentElements(2));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, meshGroup2d.addAdjacentElements(-3));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, meshGroup1d.addAdjacentElements(1));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, meshGroup1d.addAdjacentElements(-2));
	// 1-D is not yet implemented as there are no face maps to work off
	EXPECT_EQ(RESULT_ERROR_NOT_IMPLEMENTED, meshGroup1d.addAdjacentElements(0));

	const int expectedElementIdentifiers0[] = {
		64, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 17, 18, 20, 22, 23, 24, 26, 27,
		30, 31, 32, 35, 36, 39, 41, 44, 45, 47, 50, 51, 52, 60, 61, 62, 64, 65, 67,
		69, 71, 72, 73, 74, 77, 78, 80, 81, 82, 83, 84, 86, 87, 88, 89, 90, 92, 93,
		94, 95, 96, 98, 100, 101 };
	const int expectedElementIdentifiers1[] = {
		18, 2, 14, 18, 22, 30, 32, 36, 45, 50, 52, 61, 67, 77, 82, 88, 93, 94, 98 };
	const int expectedElementIdentifiers2[] = { 5, 2, 30, 61, 77, 93 };
	const int sharedDimensions[6] = { 2, -1, 1, -2, 0, -3 };
	for (int i = 0; i < 6; ++i)
	{
		const int sharedDimension = sharedDimensions[i];
		const int absoluteSharedDimension = (sharedDimension < 0) ? (3 + sharedDimension) : sharedDimension;
		FieldGroup group = zinc.fm.createFieldGroup();
		EXPECT_TRUE(group.isValid());
		EXPECT_EQ(RESULT_OK, group.setName((std::string("adjacent") + std::to_string(sharedDimension)).c_str()));
		EXPECT_EQ(RESULT_OK, group.setSubelementHandlingMode(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL));
		EXPECT_EQ(RESULT_OK, group.setManaged(true));
		MeshGroup meshGroup3d = group.createMeshGroup(mesh3d);
		EXPECT_TRUE(meshGroup3d.isValid());
		EXPECT_EQ(RESULT_OK, meshGroup3d.addElement(element93));
		EXPECT_EQ(1, meshGroup3d.getSize());
		EXPECT_EQ(RESULT_OK, meshGroup3d.addAdjacentElements(sharedDimension));
		const int* expectedElementIdentifiers =
			(absoluteSharedDimension == 0) ? expectedElementIdentifiers0 :
			(absoluteSharedDimension == 1) ? expectedElementIdentifiers1 :
			expectedElementIdentifiers2;
		ASSERT_EQ(expectedElementIdentifiers[0], meshGroup3d.getSize());
		Elementiterator iter = meshGroup3d.createElementiterator();
		Element element;
		int e = 1;
		while ((element = iter.next()).isValid())
		{
			const int elementIdentifier = element.getIdentifier();
			EXPECT_EQ(expectedElementIdentifiers[e], elementIdentifier);
			++e;
		}
	}
}

TEST(ZincFieldGroup, trianglemesh_addAdjacentElements)
{
	ZincTestSetupCpp zinc;

	EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(resourcePath("fieldio/triangle_mesh_1371.exf").c_str()));

	EXPECT_EQ(RESULT_OK, zinc.fm.defineAllFaces());
	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	EXPECT_EQ(37, mesh2d.getSize());
	Mesh mesh1d = zinc.fm.findMeshByDimension(1);
	EXPECT_EQ(66, mesh1d.getSize());
	Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_EQ(30, nodes.getSize());

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());
	EXPECT_EQ(RESULT_OK, group.setName("original"));
	EXPECT_EQ(RESULT_OK, group.setSubelementHandlingMode(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL));
	EXPECT_EQ(RESULT_OK, group.setManaged(true));
	Element element22 = mesh2d.findElementByIdentifier(22);
	MeshGroup meshGroup2d = group.createMeshGroup(mesh2d);
	EXPECT_TRUE(meshGroup2d.isValid());
	EXPECT_EQ(RESULT_OK, meshGroup2d.addElement(element22));
	EXPECT_EQ(1, meshGroup2d.getSize());
	EXPECT_EQ(3, group.getMeshGroup(mesh1d).getSize());

	const int expectedElementIdentifiers0[] = {
		14, 2, 3, 7, 8, 11, 14, 17, 18, 19, 20, 21, 22, 24, 26 };
	const int expectedElementIdentifiers1[] = { 4, 3, 17, 20, 22 };
	const int sharedDimensions[4] = { 1, -1, 0, -2 };
	for (int i = 0; i < 4; ++i)
	{
		const int sharedDimension = sharedDimensions[i];
		const int absoluteSharedDimension = (sharedDimension < 0) ? (2 + sharedDimension) : sharedDimension;
		FieldGroup group = zinc.fm.createFieldGroup();
		EXPECT_TRUE(group.isValid());
		EXPECT_EQ(RESULT_OK, group.setName((std::string("adjacent") + std::to_string(sharedDimension)).c_str()));
		EXPECT_EQ(RESULT_OK, group.setSubelementHandlingMode(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL));
		EXPECT_EQ(RESULT_OK, group.setManaged(true));
		MeshGroup meshGroup2d = group.createMeshGroup(mesh2d);
		EXPECT_TRUE(meshGroup2d.isValid());
		EXPECT_EQ(RESULT_OK, meshGroup2d.addElement(element22));
		EXPECT_EQ(1, meshGroup2d.getSize());
		EXPECT_EQ(RESULT_OK, meshGroup2d.addAdjacentElements(sharedDimension));
		const int* expectedElementIdentifiers =
			(absoluteSharedDimension == 0) ? expectedElementIdentifiers0 :
			expectedElementIdentifiers1;
		ASSERT_EQ(expectedElementIdentifiers[0], meshGroup2d.getSize());
		Elementiterator iter = meshGroup2d.createElementiterator();
		Element element;
		int e = 1;
		while ((element = iter.next()).isValid())
		{
			const int elementIdentifier = element.getIdentifier();
			EXPECT_EQ(expectedElementIdentifiers[e], elementIdentifier);
			++e;
		}
	}
}
