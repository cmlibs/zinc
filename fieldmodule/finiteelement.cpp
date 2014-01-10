/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include "zinctestsetup.hpp"
#include <zinc/core.h>
#include <zinc/element.h>
#include <zinc/field.h>
#include <zinc/fieldcache.h>
#include <zinc/fieldfiniteelement.h>
#include <zinc/node.h>
#include <zinc/stream.h>

#include "zinctestsetupcpp.hpp"
#include <zinc/context.hpp>
#include <zinc/element.hpp>
#include <zinc/field.hpp>
#include <zinc/fieldcache.hpp>
#include <zinc/fieldmodule.hpp>
#include <zinc/fieldfiniteelement.hpp>
#include <zinc/node.hpp>
#include <zinc/region.hpp>
#include <zinc/status.hpp>

#include "test_resources.h"

TEST(cmzn_field_finite_element, create)
{
	ZincTestSetup zinc;
	int result;

	cmzn_field_id field = cmzn_fieldmodule_create_field_finite_element(zinc.fm, /*number_of_components*/2);
	EXPECT_NE(static_cast<cmzn_field_id>(0), field);
	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_name(field, "coordinates"));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_type_coordinate(field, true));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_managed(field, true));

	cmzn_field_finite_element_id feField = cmzn_field_cast_finite_element(field);
	EXPECT_NE(static_cast<cmzn_field_finite_element_id>(0), feField);
	cmzn_field_finite_element_destroy(&feField);

	char *componentName = cmzn_field_get_component_name(field, 1);
	EXPECT_STREQ("1", componentName);
	cmzn_deallocate(componentName);
	EXPECT_EQ(2, result = cmzn_field_get_number_of_components(field));
	EXPECT_TRUE(cmzn_field_is_managed(field));

	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_component_name(field, 1, "x"));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_component_name(field, 2, "y"));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_field_set_component_name(0, 1, "A"));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_field_set_component_name(field, 0, "A"));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_field_set_component_name(field, 3, "A"));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_field_set_component_name(field, 1, 0));

	componentName = cmzn_field_get_component_name(field, 1);
	EXPECT_STREQ("x", componentName);
	cmzn_deallocate(componentName);
	componentName = cmzn_field_get_component_name(field, 2);
	EXPECT_STREQ("y", componentName);
	cmzn_deallocate(componentName);

	cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(zinc.fm, CMZN_FIELD_DOMAIN_TYPE_NODES);
	EXPECT_NE(static_cast<cmzn_nodeset_id>(0), nodeset);
	cmzn_nodetemplate_id nodetemplate = cmzn_nodeset_create_nodetemplate(nodeset);
	EXPECT_NE(static_cast<cmzn_nodetemplate_id>(0), nodetemplate);
	EXPECT_EQ(CMZN_OK, result = cmzn_nodetemplate_define_field(nodetemplate, field));

	double nodeCoordinates[8] =
	{
		0.0, 0.0,
		1.0, 0.0,
		0.0, 1.0,
		1.0, 1.0
	};
	cmzn_fieldcache_id cache = cmzn_fieldmodule_create_fieldcache(zinc.fm);
	EXPECT_NE(static_cast<cmzn_fieldcache_id>(0), cache);
	for (int i = 1; i <= 4; ++i)
	{
		cmzn_node_id node = cmzn_nodeset_create_node(nodeset, -1, nodetemplate);
		EXPECT_NE(static_cast<cmzn_node_id>(0), node);
		EXPECT_EQ(i, result = cmzn_node_get_identifier(node));
		EXPECT_EQ(CMZN_OK, result = cmzn_fieldcache_set_node(cache, node));
		EXPECT_EQ(CMZN_OK, result = cmzn_field_assign_real(field, cache, 2, nodeCoordinates + (i - 1)*2));
		EXPECT_EQ(CMZN_OK, result = cmzn_node_destroy(&node));
	}
	cmzn_fieldcache_destroy(&cache);
	cmzn_nodetemplate_destroy(&nodetemplate);

	cmzn_mesh_id mesh = cmzn_fieldmodule_find_mesh_by_dimension(zinc.fm, 2);
	EXPECT_NE(static_cast<cmzn_mesh_id>(0), mesh);
	cmzn_elementtemplate_id elementtemplate = cmzn_mesh_create_elementtemplate(mesh);
	EXPECT_NE(static_cast<cmzn_elementtemplate_id>(0), elementtemplate);
	EXPECT_EQ(CMZN_OK, result = cmzn_elementtemplate_set_element_shape_type(elementtemplate, CMZN_ELEMENT_SHAPE_TYPE_SQUARE));
	EXPECT_EQ(CMZN_OK, result = cmzn_elementtemplate_set_number_of_nodes(elementtemplate, 4));

	cmzn_elementbasis_id basis = cmzn_fieldmodule_create_elementbasis(zinc.fm, 2, CMZN_ELEMENTBASIS_FUNCTION_TYPE_LINEAR_LAGRANGE);
	EXPECT_NE(static_cast<cmzn_elementbasis_id>(0), basis);
	int localNodeIndexes[4] = { 1, 2, 3, 4 };
	EXPECT_EQ(CMZN_OK, result = cmzn_elementtemplate_define_field_simple_nodal(
		elementtemplate, field, /*component_number*/-1, basis, 4, localNodeIndexes));

	// for complex reasons you need to set the element nodes
	// in the template before creating each element
	for (int i = 1; i <= 4; ++i)
	{
		cmzn_node_id node = cmzn_nodeset_find_node_by_identifier(nodeset, i);
		EXPECT_NE(static_cast<cmzn_node_id>(0), node);
		EXPECT_EQ(CMZN_OK, result = cmzn_elementtemplate_set_node(elementtemplate, i, node));
		EXPECT_EQ(CMZN_OK, result = cmzn_node_destroy(&node));
	}
	cmzn_element_id element = cmzn_mesh_create_element(mesh, -1, elementtemplate);
	EXPECT_NE(static_cast<cmzn_element_id>(0), element);
	cmzn_element_destroy(&element);
	
	EXPECT_EQ(4, result = cmzn_nodeset_get_size(nodeset));
	EXPECT_EQ(1, result = cmzn_mesh_get_size(mesh));

	EXPECT_EQ(CMZN_OK, result = cmzn_fieldmodule_define_all_faces(zinc.fm));
	cmzn_mesh_id lineMesh = cmzn_fieldmodule_find_mesh_by_dimension(zinc.fm, 1);
	EXPECT_NE(static_cast<cmzn_mesh_id>(0), lineMesh);
	EXPECT_EQ(4, result = cmzn_mesh_get_size(lineMesh));
	cmzn_mesh_destroy(&lineMesh);

	//EXPECT_EQ(CMZN_OK, result = cmzn_region_write_file(zinc.root_region, "km.exelem"));

	EXPECT_EQ(CMZN_OK, result = cmzn_elementbasis_destroy(&basis));
	EXPECT_EQ(CMZN_OK, result = cmzn_elementtemplate_destroy(&elementtemplate));

	EXPECT_EQ(CMZN_OK, result = cmzn_nodeset_destroy(&nodeset));
	EXPECT_EQ(CMZN_OK, result = cmzn_mesh_destroy(&mesh));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_destroy(&field));
}

TEST(ZincFieldFiniteElement, create)
{
	ZincTestSetupCpp zinc;
	int result;

	FieldFiniteElement field = zinc.fm.createFieldFiniteElement(/*numberOfComponents*/2);
	EXPECT_TRUE(field.isValid());
	EXPECT_EQ(CMZN_OK, result = field.setName("coordinates"));
	EXPECT_EQ(CMZN_OK, result = field.setTypeCoordinate(true));
	EXPECT_EQ(CMZN_OK, result = field.setManaged(true));

	Field tmp = field;
	FieldFiniteElement feField = tmp.castFiniteElement();
	EXPECT_TRUE(feField.isValid());

	char *componentName = field.getComponentName(1);
	EXPECT_STREQ("1", componentName);
	cmzn_deallocate(componentName);
	EXPECT_EQ(2, result = field.getNumberOfComponents());
	EXPECT_TRUE(field.isManaged());

	EXPECT_EQ(CMZN_OK, result = field.setComponentName(1, "x"));
	EXPECT_EQ(CMZN_OK, result = field.setComponentName(2, "y"));
	Field noField;
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = noField.setComponentName(1, "A"));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = field.setComponentName(0, "A"));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = field.setComponentName(3, "A"));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = field.setComponentName(1, 0));

	componentName = field.getComponentName(1);
	EXPECT_STREQ("x", componentName);
	cmzn_deallocate(componentName);
	componentName = field.getComponentName(2);
	EXPECT_STREQ("y", componentName);
	cmzn_deallocate(componentName);

	Nodeset nodeset = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodeset.isValid());
	Nodetemplate nodetemplate = nodeset.createNodetemplate();
	EXPECT_TRUE(nodetemplate.isValid());
	EXPECT_EQ(CMZN_OK, result = nodetemplate.defineField(field));

	double nodeCoordinates[8] =
	{
		0.0, 0.0,
		1.0, 0.0,
		0.0, 1.0,
		1.0, 1.0
	};
	Fieldcache cache = zinc.fm.createFieldcache();
	EXPECT_TRUE(cache.isValid());
	for (int i = 1; i <= 4; ++i)
	{
		Node node = nodeset.createNode(-1, nodetemplate);
		EXPECT_TRUE(node.isValid());
		EXPECT_EQ(i, result = node.getIdentifier());
		EXPECT_EQ(CMZN_OK, result = cache.setNode(node));
		EXPECT_EQ(CMZN_OK, result = field.assignReal(cache, 2, nodeCoordinates + (i - 1)*2));
	}

	Mesh mesh = zinc.fm.findMeshByDimension(2);
	EXPECT_TRUE(mesh.isValid());
	Elementtemplate elementtemplate = mesh.createElementtemplate();
	EXPECT_TRUE(elementtemplate.isValid());
	EXPECT_EQ(CMZN_OK, result = elementtemplate.setElementShapeType(Element::SHAPE_TYPE_SQUARE));
	EXPECT_EQ(CMZN_OK, result = elementtemplate.setNumberOfNodes(4));

	Elementbasis basis = zinc.fm.createElementbasis(2, Elementbasis::FUNCTION_TYPE_LINEAR_LAGRANGE);
	EXPECT_TRUE(basis.isValid());
	int localNodeIndexes[4] = { 1, 2, 3, 4 };
	EXPECT_EQ(CMZN_OK, result = elementtemplate.defineFieldSimpleNodal(
		field, /*component_number*/-1, basis, 4, localNodeIndexes));

	// for complex reasons you need to set the element nodes
	// in the template before creating each element
	for (int i = 1; i <= 4; ++i)
	{
		Node node = nodeset.findNodeByIdentifier(i);
		EXPECT_TRUE(node.isValid());
		EXPECT_EQ(CMZN_OK, result = elementtemplate.setNode(i, node));
	}
	Element element = mesh.createElement(-1, elementtemplate);
	EXPECT_TRUE(element.isValid());
	
	EXPECT_EQ(4, result = nodeset.getSize());
	EXPECT_EQ(1, result = mesh.getSize());

	EXPECT_EQ(CMZN_OK, result = zinc.fm.defineAllFaces());
	Mesh lineMesh = zinc.fm.findMeshByDimension(1);
	EXPECT_TRUE(lineMesh.isValid());
	EXPECT_EQ(4, result = lineMesh.getSize());

	//EXPECT_EQ(CMZN_OK, result = zinc.root_region.writeFile("km2.exelem"));
}

TEST(cmzn_field_finite_element, create_prolate_spheroidal)
{
	ZincTestSetup zinc;
	int result;
	double value;

	cmzn_field_id field = cmzn_fieldmodule_create_field_finite_element(zinc.fm, /*number_of_components*/3);
	EXPECT_NE(static_cast<cmzn_field_id>(0), field);
	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_name(field, "coordinates"));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_component_name(field, 1, "lambda"));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_component_name(field, 2, "mu"));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_component_name(field, 3, "theta"));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_type_coordinate(field, true));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_coordinate_system_type(field, CMZN_FIELD_COORDINATE_SYSTEM_TYPE_PROLATE_SPHEROIDAL));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_coordinate_system_focus(field, 35.5));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_managed(field, true));

	EXPECT_TRUE(cmzn_field_is_type_coordinate(field));
	EXPECT_EQ(CMZN_FIELD_COORDINATE_SYSTEM_TYPE_PROLATE_SPHEROIDAL, cmzn_field_get_coordinate_system_type(field));
	ASSERT_DOUBLE_EQ(35.5, value = cmzn_field_get_coordinate_system_focus(field));
	EXPECT_TRUE(cmzn_field_is_managed(field));
	
	EXPECT_EQ(CMZN_OK, result = cmzn_field_destroy(&field));
}

TEST(ZincFieldFiniteElement, createProlateSpheroidal)
{
	ZincTestSetupCpp zinc;
	int result;
	double value;

	FieldFiniteElement field = zinc.fm.createFieldFiniteElement(/*numberOfComponents*/3);
	EXPECT_TRUE(field.isValid());
	EXPECT_EQ(CMZN_OK, result = field.setName("coordinates"));
	EXPECT_EQ(CMZN_OK, result = field.setComponentName(1, "lambda"));
	EXPECT_EQ(CMZN_OK, result = field.setComponentName(2, "mu"));
	EXPECT_EQ(CMZN_OK, result = field.setComponentName(3, "theta"));
	EXPECT_EQ(CMZN_OK, result = field.setTypeCoordinate(true));
	EXPECT_EQ(CMZN_OK, result = field.setCoordinateSystemType(Field::COORDINATE_SYSTEM_TYPE_PROLATE_SPHEROIDAL));
	EXPECT_EQ(CMZN_OK, result = field.setCoordinateSystemFocus(35.5));
	EXPECT_EQ(CMZN_OK, result = field.setManaged(true));

	EXPECT_TRUE(field.isTypeCoordinate());
	EXPECT_EQ(Field::COORDINATE_SYSTEM_TYPE_PROLATE_SPHEROIDAL, field.getCoordinateSystemType());
	ASSERT_DOUBLE_EQ(35.5, value = field.getCoordinateSystemFocus());
	EXPECT_TRUE(field.isManaged());
}

TEST(ZincFieldFiniteElement, node_value_label)
{
	ZincTestSetupCpp zinc;
	int result;

	FieldFiniteElement coordinateField = zinc.fm.createFieldFiniteElement(3);
	EXPECT_TRUE(coordinateField.isValid());
	EXPECT_EQ(OK, result = coordinateField.setManaged(true));
	EXPECT_EQ(OK, result = coordinateField.setName("coordinates"));
	EXPECT_EQ(OK, result = coordinateField.setTypeCoordinate(true));

	Nodeset nodeset = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodeset.isValid());

	Nodetemplate nodeTemplate = nodeset.createNodetemplate();
	EXPECT_TRUE(nodeTemplate.isValid());
	EXPECT_EQ(OK, nodeTemplate.defineField(coordinateField));
	EXPECT_EQ(1, result = nodeTemplate.getValueNumberOfVersions(coordinateField, /*componentNumber*/-1, Node::VALUE_LABEL_VALUE));
	EXPECT_EQ(OK, result = nodeTemplate.setValueNumberOfVersions(coordinateField, /*componentNumber*/-1, Node::VALUE_LABEL_VALUE, 1));
	EXPECT_EQ(0, result = nodeTemplate.getValueNumberOfVersions(coordinateField, /*componentNumber*/-1, Node::VALUE_LABEL_D_DS1));
	EXPECT_EQ(OK, nodeTemplate.setValueNumberOfVersions(coordinateField, /*componentNumber*/-1, Node::VALUE_LABEL_D_DS1, 2));
	EXPECT_EQ(2, result = nodeTemplate.getValueNumberOfVersions(coordinateField, /*componentNumber*/-1, Node::VALUE_LABEL_D_DS1));
	EXPECT_EQ(0, result = nodeTemplate.getValueNumberOfVersions(coordinateField, /*componentNumber*/-1, Node::VALUE_LABEL_D_DS2));
	EXPECT_EQ(OK, nodeTemplate.setValueNumberOfVersions(coordinateField, /*componentNumber*/-1, Node::VALUE_LABEL_D_DS2, 1));
	// following is 2 not 1, due to limitation that all values/derivatives have same number of versions
	EXPECT_EQ(2, result = nodeTemplate.getValueNumberOfVersions(coordinateField, /*componentNumber*/-1, Node::VALUE_LABEL_D_DS2));
	// test clearing the number of versions
	EXPECT_EQ(OK, nodeTemplate.setValueNumberOfVersions(coordinateField, /*componentNumber*/-1, Node::VALUE_LABEL_D_DS2, 0));
	EXPECT_EQ(0, result = nodeTemplate.getValueNumberOfVersions(coordinateField, /*componentNumber*/-1, Node::VALUE_LABEL_D_DS2));
	EXPECT_EQ(OK, nodeTemplate.setValueNumberOfVersions(coordinateField, /*componentNumber*/-1, Node::VALUE_LABEL_D_DS2, 1));

	FieldNodeValue dx_ds1_v1 = zinc.fm.createFieldNodeValue(coordinateField, Node::VALUE_LABEL_D_DS1, 1);
	EXPECT_TRUE(dx_ds1_v1.isValid());
	FieldNodeValue dx_ds1_v2 = zinc.fm.createFieldNodeValue(coordinateField, Node::VALUE_LABEL_D_DS1, 2);
	EXPECT_TRUE(dx_ds1_v2.isValid());
	FieldNodeValue dx_ds2 = zinc.fm.createFieldNodeValue(coordinateField, Node::VALUE_LABEL_D_DS2, 1);
	EXPECT_TRUE(dx_ds2.isValid());

	Node node = nodeset.createNode(1, nodeTemplate);
	double coordinates[3] = { 1.0, 2.0, 3.0 };
	double derivatives1_v1[3] = { 0.1, 0.2, 0.4 };
	double derivatives1_v2[3] = { 0.6, 0.5, 0.4 };
	double derivatives2[3] = { -0.1, -0.2, -0.4 };
	{
		// assign in temporary cache so later we don't evaluate the value from cache
		Fieldcache tmpCache = zinc.fm.createFieldcache();
		EXPECT_EQ(OK, result = tmpCache.setNode(node));
		EXPECT_EQ(OK, result = coordinateField.assignReal(tmpCache, 3, coordinates));
		EXPECT_EQ(OK, result = dx_ds1_v1.assignReal(tmpCache, 3, derivatives1_v1));
		EXPECT_EQ(OK, result = dx_ds1_v2.assignReal(tmpCache, 3, derivatives1_v2));
		EXPECT_EQ(OK, result = dx_ds2.assignReal(tmpCache, 3, derivatives2));
	}
	Fieldcache cache = zinc.fm.createFieldcache();
	cache.setNode(node);
	double outCoordinates[3];
	double outDerivatives1_v1[3];
	double outDerivatives1_v2[3];
	double outDerivatives2[3];
	EXPECT_EQ(OK, result = coordinateField.evaluateReal(cache, 3, outCoordinates));
	ASSERT_DOUBLE_EQ(coordinates[0], outCoordinates[0]);
	ASSERT_DOUBLE_EQ(coordinates[1], outCoordinates[1]);
	ASSERT_DOUBLE_EQ(coordinates[2], outCoordinates[2]);
	EXPECT_EQ(OK, result = dx_ds1_v1.evaluateReal(cache, 3, outDerivatives1_v1));
	ASSERT_DOUBLE_EQ(derivatives1_v1[0], outDerivatives1_v1[0]);
	ASSERT_DOUBLE_EQ(derivatives1_v1[1], outDerivatives1_v1[1]);
	ASSERT_DOUBLE_EQ(derivatives1_v1[2], outDerivatives1_v1[2]);
	EXPECT_EQ(OK, result = dx_ds1_v2.evaluateReal(cache, 3, outDerivatives1_v2));
	ASSERT_DOUBLE_EQ(derivatives1_v2[0], outDerivatives1_v2[0]);
	ASSERT_DOUBLE_EQ(derivatives1_v2[1], outDerivatives1_v2[1]);
	ASSERT_DOUBLE_EQ(derivatives1_v2[2], outDerivatives1_v2[2]);
	EXPECT_EQ(OK, result = dx_ds2.evaluateReal(cache, 3, outDerivatives2));
	ASSERT_DOUBLE_EQ(derivatives2[0], outDerivatives2[0]);
	ASSERT_DOUBLE_EQ(derivatives2[1], outDerivatives2[1]);
	ASSERT_DOUBLE_EQ(derivatives2[2], outDerivatives2[2]);
}