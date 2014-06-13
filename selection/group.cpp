/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <zinc/core.h>
#include <zinc/element.h>
#include <zinc/field.h>
#include <zinc/fieldcache.h>
#include <zinc/fieldcomposite.h>
#include <zinc/fieldconstant.h>
#include <zinc/fieldfiniteelement.h>
#include <zinc/fieldgroup.h>
#include <zinc/fieldlogicaloperators.h>
#include <zinc/fieldsubobjectgroup.h>
#include <zinc/node.h>
#include <zinc/region.h>

#include <zinc/element.hpp>
#include <zinc/field.hpp>
#include <zinc/fieldcache.hpp>
#include <zinc/fieldcomposite.hpp>
#include <zinc/fieldconstant.hpp>
#include <zinc/fieldfiniteelement.hpp>
#include <zinc/fieldgroup.hpp>
#include <zinc/fieldlogicaloperators.hpp>
#include <zinc/fieldsubobjectgroup.hpp>
#include <zinc/node.hpp>
#include <zinc/region.hpp>

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
	EXPECT_FALSE(cmzn_nodeset_contains_node(cmzn_nodeset_group_base_cast(nodesetGroup), node));
	EXPECT_EQ(CMZN_OK, cmzn_nodeset_group_add_node(nodesetGroup, node));
	EXPECT_TRUE(cmzn_nodeset_contains_node(cmzn_nodeset_group_base_cast(nodesetGroup), node));

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
	EXPECT_FALSE(cmzn_nodeset_contains_node(cmzn_nodeset_group_base_cast(nodesetGroup), node));

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
	EXPECT_FALSE(cmzn_mesh_contains_element(cmzn_mesh_group_base_cast(meshGroup), element));
	EXPECT_EQ(CMZN_OK, cmzn_mesh_group_add_element(meshGroup, element));
	EXPECT_TRUE(cmzn_mesh_contains_element(cmzn_mesh_group_base_cast(meshGroup), element));

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
	EXPECT_FALSE(cmzn_mesh_contains_element(cmzn_mesh_group_base_cast(meshGroup), element));

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

TEST(ZincFieldElementGroup, add_remove_conditional)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDMODULE_TWO_CUBES_RESOURCE)));

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());

	int size1, size2, size3;
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_EQ(2, size3 = mesh3d.getSize());
	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	EXPECT_EQ(11, size2 = mesh2d.getSize());
	Mesh mesh1d = zinc.fm.findMeshByDimension(1);
	EXPECT_EQ(20, size1 = mesh1d.getSize());

	FieldElementGroup facesGroup = group.createFieldElementGroup(mesh2d);
	EXPECT_TRUE(facesGroup.isValid());
	MeshGroup facesMeshGroup = facesGroup.getMeshGroup();
	EXPECT_TRUE(facesMeshGroup.isValid());
	EXPECT_EQ(0, facesMeshGroup.getSize());

	EXPECT_EQ(ERROR_ARGUMENT, facesMeshGroup.addElementsConditional(Field()));
	EXPECT_EQ(ERROR_ARGUMENT, facesMeshGroup.removeElementsConditional(Field()));

	FieldElementGroup linesGroup = group.createFieldElementGroup(mesh1d);
	EXPECT_TRUE(linesGroup.isValid());
	MeshGroup linesMeshGroup = linesGroup.getMeshGroup();
	EXPECT_TRUE(linesMeshGroup.isValid());
	EXPECT_EQ(0, linesMeshGroup.getSize());

	FieldIsOnFace isOnFaceField = zinc.fm.createFieldIsOnFace(Element::FACE_TYPE_XI1_0);
	EXPECT_TRUE(isOnFaceField.isValid());

	const double oneValue = 1.0;
	FieldConstant trueField = zinc.fm.createFieldConstant(1, &oneValue);
	EXPECT_TRUE(trueField.isValid());

	Element element1 = mesh2d.findElementByIdentifier(1);
	Element element2 = mesh2d.findElementByIdentifier(2);

	EXPECT_EQ(OK, facesMeshGroup.addElementsConditional(isOnFaceField));
	EXPECT_EQ(2, result = facesMeshGroup.getSize());
	EXPECT_TRUE(facesMeshGroup.containsElement(element1));
	EXPECT_TRUE(facesMeshGroup.containsElement(element2));

	EXPECT_EQ(0, linesMeshGroup.getSize());

	// copy into another element group to test conditional add/removal
	// optimisation for smaller groups
	FieldElementGroup otherFacesGroup = zinc.fm.createFieldElementGroup(mesh2d);
	EXPECT_TRUE(otherFacesGroup.isValid());
	MeshGroup otherFacesMeshGroup = otherFacesGroup.getMeshGroup();
	EXPECT_TRUE(otherFacesMeshGroup.isValid());
	EXPECT_EQ(OK, otherFacesMeshGroup.addElementsConditional(facesGroup));
	EXPECT_EQ(2, result = otherFacesMeshGroup.getSize());
	EXPECT_TRUE(otherFacesMeshGroup.containsElement(element1));
	EXPECT_TRUE(otherFacesMeshGroup.containsElement(element2));

	// add all faces for conditional removal
	EXPECT_EQ(OK, facesMeshGroup.addElementsConditional(trueField));
	EXPECT_EQ(11, result = facesMeshGroup.getSize());

	EXPECT_EQ(OK, facesMeshGroup.removeElementsConditional(isOnFaceField));
	EXPECT_EQ(9, result = facesMeshGroup.getSize());
	EXPECT_FALSE(facesMeshGroup.containsElement(element1));
	EXPECT_FALSE(facesMeshGroup.containsElement(element2));

	// test using self as a conditional
	EXPECT_EQ(OK, facesMeshGroup.removeElementsConditional(facesGroup));
	EXPECT_EQ(0, facesMeshGroup.getSize());

	EXPECT_EQ(OK, facesMeshGroup.addElementsConditional(trueField));
	EXPECT_EQ(11, result = facesMeshGroup.getSize());

	EXPECT_EQ(OK, facesMeshGroup.removeElementsConditional(otherFacesGroup));
	EXPECT_EQ(9, result = facesMeshGroup.getSize());
	EXPECT_FALSE(facesMeshGroup.containsElement(element1));
	EXPECT_FALSE(facesMeshGroup.containsElement(element2));

	// check not an error to remove nodes already removed
	EXPECT_EQ(OK, facesMeshGroup.removeAllElements());
	EXPECT_EQ(0, result = facesMeshGroup.getSize());
	EXPECT_EQ(OK, facesMeshGroup.removeElementsConditional(otherFacesGroup));
}

TEST(ZincFieldNodeGroup, add_remove_conditional)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDMODULE_TWO_CUBES_RESOURCE)));

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());

	int size;
	Nodeset nodeset = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_EQ(12, size = nodeset.getSize());

	FieldNodeGroup nodesGroup = group.createFieldNodeGroup(nodeset);
	EXPECT_TRUE(nodesGroup.isValid());
	NodesetGroup nodesetGroup = nodesGroup.getNodesetGroup();
	EXPECT_TRUE(nodesetGroup.isValid());
	EXPECT_EQ(0, nodesetGroup.getSize());

	EXPECT_EQ(ERROR_ARGUMENT, nodesetGroup.addNodesConditional(Field()));
	EXPECT_EQ(ERROR_ARGUMENT, nodesetGroup.removeNodesConditional(Field()));

	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	FieldComponent x = zinc.fm.createFieldComponent(coordinates, 1);
	EXPECT_TRUE(x.isValid());
	const double fifteen = 15.0;
	FieldGreaterThan x_gt_15 = x > zinc.fm.createFieldConstant(1, &fifteen);
	EXPECT_TRUE(x_gt_15.isValid());

	const double oneValue = 1.0;
	FieldConstant trueField = zinc.fm.createFieldConstant(1, &oneValue);
	EXPECT_TRUE(trueField.isValid());

	EXPECT_EQ(OK, nodesetGroup.addNodesConditional(x_gt_15));
	EXPECT_EQ(4, result = nodesetGroup.getSize());
	for (int i = 3; i <= 12; i += 3)
		EXPECT_TRUE(nodesetGroup.containsNode(nodeset.findNodeByIdentifier(i)));

	// copy into another element group to test conditional add/removal
	// optimisation for smaller groups
	FieldNodeGroup otherNodesGroup = zinc.fm.createFieldNodeGroup(nodeset);
	EXPECT_TRUE(otherNodesGroup.isValid());
	NodesetGroup otherNodesetGroup = otherNodesGroup.getNodesetGroup();
	EXPECT_TRUE(otherNodesetGroup.isValid());
	EXPECT_EQ(OK, otherNodesetGroup.addNodesConditional(nodesGroup));
	EXPECT_EQ(4, result = otherNodesetGroup.getSize());
	for (int i = 3; i <= 12; i += 3)
		EXPECT_TRUE(otherNodesetGroup.containsNode(nodeset.findNodeByIdentifier(i)));

	// add all faces for conditional removal
	EXPECT_EQ(OK, nodesetGroup.addNodesConditional(trueField));
	EXPECT_EQ(12, result = nodesetGroup.getSize());

	EXPECT_EQ(OK, nodesetGroup.removeNodesConditional(x_gt_15));
	EXPECT_EQ(8, result = nodesetGroup.getSize());
	for (int i = 3; i <= 12; i += 3)
		EXPECT_FALSE(nodesetGroup.containsNode(nodeset.findNodeByIdentifier(i)));

	// test using self as a conditional
	EXPECT_EQ(OK, nodesetGroup.removeNodesConditional(nodesGroup));
	EXPECT_EQ(0, nodesetGroup.getSize());

	EXPECT_EQ(OK, nodesetGroup.addNodesConditional(trueField));
	EXPECT_EQ(12, result = nodesetGroup.getSize());

	EXPECT_EQ(OK, nodesetGroup.removeNodesConditional(otherNodesGroup));
	EXPECT_EQ(8, result = nodesetGroup.getSize());
	for (int i = 3; i <= 12; i += 3)
		EXPECT_FALSE(nodesetGroup.containsNode(nodeset.findNodeByIdentifier(i)));

	// check not an error to remove nodes already removed
	EXPECT_EQ(OK, nodesetGroup.removeAllNodes());
	EXPECT_EQ(0, result = nodesetGroup.getSize());
	EXPECT_EQ(OK, nodesetGroup.removeNodesConditional(otherNodesGroup));
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
	EXPECT_EQ(OK, result = group.setSubelementHandlingMode(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL));
	EXPECT_EQ(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL, mode = group.getSubelementHandlingMode());

	// check subelement handling mode is inherited by new subregion groups
	FieldGroup childGroup = group.createSubregionFieldGroup(childRegion);
	EXPECT_TRUE(childGroup.isValid());
	EXPECT_EQ(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL, mode = childGroup.getSubelementHandlingMode());

	EXPECT_EQ(OK, result = group.setSubelementHandlingMode(FieldGroup::SUBELEMENT_HANDLING_MODE_NONE));
	EXPECT_EQ(FieldGroup::SUBELEMENT_HANDLING_MODE_NONE, mode = childGroup.getSubelementHandlingMode());

	// orphan child group, set new handling mode, rediscover child group and check mode
	EXPECT_EQ(OK, result = group.clear());
	EXPECT_EQ(OK, result = group.removeEmptySubgroups());
	EXPECT_EQ(OK, result = group.setSubelementHandlingMode(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL));
	EXPECT_EQ(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL, mode = group.getSubelementHandlingMode());
	EXPECT_EQ(FieldGroup::SUBELEMENT_HANDLING_MODE_NONE, mode = childGroup.getSubelementHandlingMode());
	// rediscovery of subregion and subobject groups are inconsistent
	// former works on create, latter on find. Create makes more sense.
	FieldGroup tempGroup = group.getSubregionFieldGroup(childRegion);
	EXPECT_FALSE(tempGroup.isValid());
	tempGroup = group.createSubregionFieldGroup(childRegion);
	EXPECT_TRUE(tempGroup.isValid());
	EXPECT_EQ(childGroup, tempGroup);
	EXPECT_EQ(FieldGroup::SUBELEMENT_HANDLING_MODE_NONE, mode = childGroup.getSubelementHandlingMode());
}

TEST(ZincFieldElementGroup, add_with_subelement_handling)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDMODULE_TWO_CUBES_RESOURCE)));

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());
	FieldGroup::SubelementHandlingMode mode;
	EXPECT_EQ(OK, result = group.setSubelementHandlingMode(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL));
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

	FieldElementGroup elementsGroup = group.createFieldElementGroup(mesh3d);
	EXPECT_TRUE(elementsGroup.isValid());
	MeshGroup elementsMeshGroup = elementsGroup.getMeshGroup();
	EXPECT_TRUE(elementsMeshGroup.isValid());
	EXPECT_EQ(0, elementsMeshGroup.getSize());

	FieldElementGroup facesGroup = group.createFieldElementGroup(mesh2d);
	EXPECT_TRUE(facesGroup.isValid());
	MeshGroup facesMeshGroup = facesGroup.getMeshGroup();
	EXPECT_TRUE(facesMeshGroup.isValid());

	FieldElementGroup linesGroup = group.createFieldElementGroup(mesh1d);
	EXPECT_TRUE(linesGroup.isValid());
	MeshGroup linesMeshGroup = linesGroup.getMeshGroup();
	EXPECT_TRUE(linesMeshGroup.isValid());

	FieldNodeGroup nodesGroup = group.createFieldNodeGroup(nodeset);
	EXPECT_TRUE(nodesGroup.isValid());
	NodesetGroup nodesetGroup = nodesGroup.getNodesetGroup();
	EXPECT_TRUE(nodesetGroup.isValid());

	EXPECT_EQ(0, elementsMeshGroup.getSize());
	EXPECT_EQ(0, facesMeshGroup.getSize());
	EXPECT_EQ(0, linesMeshGroup.getSize());
	EXPECT_EQ(0, nodesetGroup.getSize());

	const double oneValue = 1.0;
	FieldConstant trueField = zinc.fm.createFieldConstant(1, &oneValue);
	EXPECT_TRUE(trueField.isValid());

	EXPECT_EQ(OK, result = elementsMeshGroup.addElementsConditional(trueField));
	EXPECT_EQ(2, size3 = elementsMeshGroup.getSize());
	EXPECT_EQ(11, size2 = facesMeshGroup.getSize());
	EXPECT_EQ(20, size1 = linesMeshGroup.getSize());
	EXPECT_EQ(12, size0 = nodesetGroup.getSize());

	// in this mode removeAllElements on top mesh should empty whole group
	// however handling subelements on remove is not implemented yet - GRC
	EXPECT_EQ(OK, result = elementsMeshGroup.removeAllElements());
	EXPECT_EQ(OK, result = facesMeshGroup.removeAllElements());
	EXPECT_EQ(OK, result = linesMeshGroup.removeAllElements());
	EXPECT_EQ(OK, result = nodesetGroup.removeAllNodes());
	EXPECT_EQ(0, elementsMeshGroup.getSize());
	EXPECT_EQ(0, facesMeshGroup.getSize());
	EXPECT_EQ(0, linesMeshGroup.getSize());
	EXPECT_EQ(0, nodesetGroup.getSize());

	EXPECT_EQ(OK, result = elementsMeshGroup.addElement(element1));
	EXPECT_EQ(1, size3 = elementsMeshGroup.getSize());
	EXPECT_EQ(6, size2 = facesMeshGroup.getSize());
	EXPECT_EQ(12, size1 = linesMeshGroup.getSize());
	EXPECT_EQ(8, size0 = nodesetGroup.getSize());
	EXPECT_EQ(OK, result = elementsMeshGroup.addElement(element2));
	EXPECT_EQ(2, size3 = elementsMeshGroup.getSize());
	EXPECT_EQ(11, size2 = facesMeshGroup.getSize());
	EXPECT_EQ(20, size1 = linesMeshGroup.getSize());
	EXPECT_EQ(12, size0 = nodesetGroup.getSize());

	EXPECT_EQ(OK, result = elementsMeshGroup.removeAllElements());
	EXPECT_EQ(OK, result = facesMeshGroup.removeAllElements());
	EXPECT_EQ(OK, result = linesMeshGroup.removeAllElements());
	EXPECT_EQ(OK, result = nodesetGroup.removeAllNodes());
	EXPECT_EQ(0, elementsMeshGroup.getSize());
	EXPECT_EQ(0, facesMeshGroup.getSize());
	EXPECT_EQ(0, linesMeshGroup.getSize());
	EXPECT_EQ(0, nodesetGroup.getSize());

	FieldIsOnFace isOnFaceField = zinc.fm.createFieldIsOnFace(Element::FACE_TYPE_XI2_0);
	EXPECT_TRUE(isOnFaceField.isValid());

	EXPECT_EQ(OK, result = facesMeshGroup.addElementsConditional(isOnFaceField));
	EXPECT_EQ(0, size3 = elementsMeshGroup.getSize());
	EXPECT_EQ(2, size2 = facesMeshGroup.getSize());
	EXPECT_EQ(7, size1 = linesMeshGroup.getSize());
	EXPECT_EQ(6, size0 = nodesetGroup.getSize());
}
