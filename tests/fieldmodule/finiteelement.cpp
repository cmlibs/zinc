/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include "zinctestsetup.hpp"
#include <opencmiss/zinc/core.h>
#include <opencmiss/zinc/element.h>
#include <opencmiss/zinc/field.h>
#include <opencmiss/zinc/fieldcache.h>
#include <opencmiss/zinc/fieldfiniteelement.h>
#include <opencmiss/zinc/node.h>
#include <opencmiss/zinc/stream.h>

#include "zinctestsetupcpp.hpp"
#include <opencmiss/zinc/context.hpp>
#include <opencmiss/zinc/element.hpp>
#include <opencmiss/zinc/field.hpp>
#include <opencmiss/zinc/fieldarithmeticoperators.hpp>
#include <opencmiss/zinc/fieldderivatives.hpp>
#include <opencmiss/zinc/fieldcache.hpp>
#include <opencmiss/zinc/fieldconstant.hpp>
#include <opencmiss/zinc/fieldmodule.hpp>
#include <opencmiss/zinc/fieldfiniteelement.hpp>
#include <opencmiss/zinc/fieldvectoroperators.hpp>
#include <opencmiss/zinc/node.hpp>
#include <opencmiss/zinc/region.hpp>
#include <opencmiss/zinc/scene.hpp>
#include <opencmiss/zinc/status.hpp>

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

	cmzn_field_id constant_field = cmzn_fieldmodule_create_field_finite_element(zinc.fm, /*number_of_components*/3);
	EXPECT_NE(static_cast<cmzn_field_id>(0), constant_field);
	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_name(constant_field, "element_constant"));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_managed(constant_field, true));

	feField = cmzn_field_cast_finite_element(constant_field);
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
		0.0, 2.0,
		1.0, 2.0
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

	EXPECT_EQ(CMZN_OK, result = cmzn_elementtemplate_define_field_element_constant(
		elementtemplate, constant_field, /*component_number*/-1));

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
	
	EXPECT_EQ(CMZN_OK, result = cmzn_fieldcache_set_element(cache, element));
	const double constantValues3In[3] = {2.0, 3.0, 4.0};
	EXPECT_EQ(CMZN_OK, result = cmzn_field_assign_real(constant_field, cache, 3, constantValues3In));

	double constantValues3Out[3] = { 5.0, 5.0, 5.0 };
	EXPECT_EQ(CMZN_OK, result = cmzn_field_evaluate_real(constant_field, cache, 3, constantValues3Out));
	EXPECT_EQ(constantValues3In[0], constantValues3Out[0]);
	EXPECT_EQ(constantValues3In[1], constantValues3Out[1]);
	EXPECT_EQ(constantValues3In[2], constantValues3Out[2]);

	EXPECT_EQ(4, result = cmzn_nodeset_get_size(nodeset));
	EXPECT_EQ(1, result = cmzn_mesh_get_size(mesh));

	EXPECT_EQ(CMZN_OK, result = cmzn_fieldmodule_define_all_faces(zinc.fm));
	cmzn_mesh_id lineMesh = cmzn_fieldmodule_find_mesh_by_dimension(zinc.fm, 1);
	EXPECT_NE(static_cast<cmzn_mesh_id>(0), lineMesh);
	EXPECT_EQ(4, result = cmzn_mesh_get_size(lineMesh));
	cmzn_mesh_destroy(&lineMesh);

	// test evaluation of field in element
	EXPECT_EQ(CMZN_OK, result = cmzn_fieldcache_clear_location(cache));
	const double xi[2] = { 0.25, 0.75 };
	EXPECT_EQ(CMZN_OK, result = cmzn_fieldcache_set_mesh_location(cache, element, 2, xi));
	double output[2];
	EXPECT_EQ(CMZN_OK, result = cmzn_field_evaluate_real(field, cache, 2, output));
	ASSERT_DOUBLE_EQ(0.25, output[0]);
	ASSERT_DOUBLE_EQ(1.5, output[1]);

	cmzn_element_destroy(&element);
	cmzn_fieldcache_destroy(&cache);
	EXPECT_EQ(CMZN_OK, result = cmzn_elementbasis_destroy(&basis));
	EXPECT_EQ(CMZN_OK, result = cmzn_elementtemplate_destroy(&elementtemplate));

	EXPECT_EQ(CMZN_OK, result = cmzn_nodeset_destroy(&nodeset));
	EXPECT_EQ(CMZN_OK, result = cmzn_mesh_destroy(&mesh));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_destroy(&field));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_destroy(&constant_field));
}

TEST(ZincFieldFiniteElement, create)
{
	ZincTestSetupCpp zinc;
	int result;

	FieldFiniteElement field = zinc.fm.createFieldFiniteElement(/*numberOfComponents*/2);
	EXPECT_TRUE(field.isValid());
	EXPECT_EQ(OK, result = field.setName("coordinates"));
	EXPECT_EQ(OK, result = field.setTypeCoordinate(true));
	EXPECT_EQ(OK, result = field.setManaged(true));

	Field tmp = field;
	FieldFiniteElement feField = tmp.castFiniteElement();
	EXPECT_TRUE(feField.isValid());

	FieldFiniteElement constant_field = zinc.fm.createFieldFiniteElement(/*numberOfComponents*/3);
	EXPECT_TRUE(constant_field.isValid());
	EXPECT_EQ(CMZN_OK, result = constant_field.setName("element_constant"));
	EXPECT_EQ(OK, result = constant_field.setManaged(true));

	tmp = constant_field;
	feField = tmp.castFiniteElement();
	EXPECT_TRUE(feField.isValid());

	char *componentName = field.getComponentName(1);
	EXPECT_STREQ("1", componentName);
	cmzn_deallocate(componentName);
	EXPECT_EQ(2, result = field.getNumberOfComponents());
	EXPECT_TRUE(field.isManaged());

	EXPECT_EQ(OK, result = field.setComponentName(1, "x"));
	EXPECT_EQ(OK, result = field.setComponentName(2, "y"));
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
	EXPECT_EQ(OK, result = nodetemplate.defineField(field));

	double nodeCoordinates[8] =
	{
		0.0, 0.0,
		1.0, 0.0,
		0.0, 2.0,
		1.0, 2.0
	};
	Fieldcache cache = zinc.fm.createFieldcache();
	EXPECT_TRUE(cache.isValid());
	for (int i = 1; i <= 4; ++i)
	{
		Node node = nodeset.createNode(-1, nodetemplate);
		EXPECT_TRUE(node.isValid());
		EXPECT_EQ(i, result = node.getIdentifier());
		EXPECT_EQ(OK, result = cache.setNode(node));
		EXPECT_EQ(OK, result = field.assignReal(cache, 2, nodeCoordinates + (i - 1)*2));
	}

	Mesh mesh = zinc.fm.findMeshByDimension(2);
	EXPECT_TRUE(mesh.isValid());
	Elementtemplate elementtemplate = mesh.createElementtemplate();
	EXPECT_TRUE(elementtemplate.isValid());
	EXPECT_EQ(OK, result = elementtemplate.setElementShapeType(Element::SHAPE_TYPE_SQUARE));
	EXPECT_EQ(OK, result = elementtemplate.setNumberOfNodes(4));

	Elementbasis basis = zinc.fm.createElementbasis(2, Elementbasis::FUNCTION_TYPE_LINEAR_LAGRANGE);
	EXPECT_TRUE(basis.isValid());
	int localNodeIndexes[4] = { 1, 2, 3, 4 };
	EXPECT_EQ(OK, result = elementtemplate.defineFieldSimpleNodal(
		field, /*component_number*/-1, basis, 4, localNodeIndexes));

	EXPECT_EQ(CMZN_OK, result = elementtemplate.defineFieldElementConstant(
		constant_field, /*component_number*/-1));

	// for complex reasons you need to set the element nodes
	// in the template before creating each element
	for (int i = 1; i <= 4; ++i)
	{
		Node node = nodeset.findNodeByIdentifier(i);
		EXPECT_TRUE(node.isValid());
		EXPECT_EQ(OK, result = elementtemplate.setNode(i, node));
	}
	Element element = mesh.createElement(-1, elementtemplate);
	EXPECT_TRUE(element.isValid());
	
	EXPECT_EQ(CMZN_OK, result = cache.setElement(element));

	const double constantValues3In[3] = { 2.0, 3.0, 4.0 };
	EXPECT_EQ(CMZN_OK, result = constant_field.assignReal(cache, 3, constantValues3In));

	double constantValues3Out[3] = { 5.0, 5.0, 5.0 };
	EXPECT_EQ(CMZN_OK, result = constant_field.evaluateReal(cache, 3, constantValues3Out));
	EXPECT_EQ(constantValues3In[0], constantValues3Out[0]);
	EXPECT_EQ(constantValues3In[1], constantValues3Out[1]);
	EXPECT_EQ(constantValues3In[2], constantValues3Out[2]);

	EXPECT_EQ(4, result = nodeset.getSize());
	EXPECT_EQ(1, result = mesh.getSize());

	EXPECT_EQ(OK, result = zinc.fm.defineAllFaces());
	Mesh lineMesh = zinc.fm.findMeshByDimension(1);
	EXPECT_TRUE(lineMesh.isValid());
	EXPECT_EQ(4, result = lineMesh.getSize());

	// test evaluation of field in element
	EXPECT_EQ(OK, result = cache.clearLocation());
	const double xi[2] = { 0.25, 0.75 };
	EXPECT_EQ(OK, result = cache.setMeshLocation(element, 2, xi));
	double output[2];
	EXPECT_EQ(OK, result = field.evaluateReal(cache, 2, output));
	ASSERT_DOUBLE_EQ(0.25, output[0]);
	ASSERT_DOUBLE_EQ(1.5, output[1]);
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
	// following is now correctly 1 since now support variable number of versions per derivative / value label
	EXPECT_EQ(1, result = nodeTemplate.getValueNumberOfVersions(coordinateField, /*componentNumber*/-1, Node::VALUE_LABEL_D_DS2));
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

TEST(ZincFieldIsExterior, evaluate3d)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDMODULE_ALLSHAPES_RESOURCE)));

	FieldIsExterior isExteriorField = zinc.fm.createFieldIsExterior();
	EXPECT_TRUE(isExteriorField.isValid());

	Fieldcache cache = zinc.fm.createFieldcache();
	double value;
	int size;

	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_EQ(6, size = mesh3d.getSize());
	for (int i = 1; i <= size; ++i)
	{
		Element element = mesh3d.findElementByIdentifier(i);
		EXPECT_TRUE(element.isValid());
		EXPECT_EQ(OK, result = cache.setElement(element));
		EXPECT_EQ(OK, result = isExteriorField.evaluateReal(cache, 1, &value));
		EXPECT_EQ(0.0, value);
	}

	const int interiorFaces[] = { 1, 3, 6, 7, 11, 15 };
	const int numInteriorFaces = sizeof(interiorFaces)/sizeof(const int);
	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	EXPECT_EQ(24, size = mesh2d.getSize());
	for (int i = 1; i <= size; ++i)
	{
		Element element = mesh2d.findElementByIdentifier(i);
		EXPECT_TRUE(element.isValid());
		EXPECT_EQ(OK, result = cache.setElement(element));
		EXPECT_EQ(OK, result = isExteriorField.evaluateReal(cache, 1, &value));
		bool expectExterior = true;
		for (int j = 0; j < numInteriorFaces; ++j)
			if (interiorFaces[j] == i)
			{
				expectExterior = false;
				break;
			}
		if (expectExterior)
			EXPECT_EQ(1.0, value);
		else
			EXPECT_EQ(0.0, value);
	}

	Mesh mesh1d = zinc.fm.findMeshByDimension(1);
	EXPECT_EQ(33, size = mesh1d.getSize());
	for (int i = 1; i <= size; ++i)
	{
		Element element = mesh1d.findElementByIdentifier(i);
		EXPECT_TRUE(element.isValid());
		EXPECT_EQ(OK, result = cache.setElement(element));
		EXPECT_EQ(OK, result = isExteriorField.evaluateReal(cache, 1, &value));
		if (i == 10)
			EXPECT_EQ(0.0, value);
		else
			EXPECT_EQ(1.0, value);
	}
}

TEST(ZincFieldIsOnFace, evaluate)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDMODULE_CUBESQUARELINE_RESOURCE)));

	Fieldcache cache = zinc.fm.createFieldcache();
	double value;

	int size1, size2, size3;
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_EQ(1, size3 = mesh3d.getSize());
	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	EXPECT_EQ(7, size2 = mesh2d.getSize());
	Mesh mesh1d = zinc.fm.findMeshByDimension(1);
	EXPECT_EQ(16, size1 = mesh1d.getSize());

	const int expectedLines[6][5] = {
		{  2,  3,  4,  7, 10 },
		{  4,  5,  8, 11, 13 },
		{  1,  3,  5,  9, 14 },
		{  6,  7,  8, 12, 15 },
		{  1,  2,  4,  6,  0 },
		{  9, 10, 11, 12,  0 } };
	int f, i, j;
	for (f = 0; f < 6; ++f)
	{
		Element::FaceType faceType = static_cast<Element::FaceType>(Element::FACE_TYPE_XI1_0 + f);
		FieldIsOnFace isOnFaceField = zinc.fm.createFieldIsOnFace(faceType);
		EXPECT_TRUE(isOnFaceField.isValid());

		for (i = 1; i <= size3; ++i)
		{
			Element element = mesh3d.findElementByIdentifier(i);
			EXPECT_TRUE(element.isValid());
			EXPECT_EQ(OK, result = cache.setElement(element));
			EXPECT_EQ(OK, result = isOnFaceField.evaluateReal(cache, 1, &value));
			EXPECT_EQ(0.0, value);
		}
		for (i = 1; i <= size2; ++i)
		{
			Element element = mesh2d.findElementByIdentifier(i);
			EXPECT_TRUE(element.isValid());
			EXPECT_EQ(OK, result = cache.setElement(element));
			EXPECT_EQ(OK, result = isOnFaceField.evaluateReal(cache, 1, &value));
			if (f + 1 == i)
				EXPECT_EQ(1.0, value);
			else
				EXPECT_EQ(0.0, value);
		}
		for (i = 1; i <= size1; ++i)
		{
			Element element = mesh1d.findElementByIdentifier(i);
			EXPECT_TRUE(element.isValid());
			EXPECT_EQ(OK, result = cache.setElement(element));
			EXPECT_EQ(OK, result = isOnFaceField.evaluateReal(cache, 1, &value));
			bool expectOnFace = false;
			for (j = 0; j < 5; ++j)
				if (expectedLines[f][j] == i)
				{
					expectOnFace = true;
					break;
				}
			if (expectOnFace)
				EXPECT_EQ(1.0, value);
			else
				EXPECT_EQ(0.0, value);
		}
	}

	FieldIsOnFace allFaceField = zinc.fm.createFieldIsOnFace(Element::FACE_TYPE_ALL);
	EXPECT_TRUE(allFaceField.isValid());
	FieldIsOnFace anyFaceField = zinc.fm.createFieldIsOnFace(Element::FACE_TYPE_ANY_FACE);
	EXPECT_TRUE(anyFaceField.isValid());
	FieldIsOnFace noFaceField = zinc.fm.createFieldIsOnFace(Element::FACE_TYPE_NO_FACE);
	EXPECT_TRUE(noFaceField.isValid());
	for (i = 1; i <= size3; ++i)
	{
		Element element = mesh3d.findElementByIdentifier(i);
		EXPECT_EQ(OK, result = cache.setElement(element));
		EXPECT_EQ(OK, result = allFaceField.evaluateReal(cache, 1, &value));
		EXPECT_EQ(1.0, value);
		EXPECT_EQ(OK, result = anyFaceField.evaluateReal(cache, 1, &value));
		EXPECT_EQ(0.0, value);
		EXPECT_EQ(OK, result = noFaceField.evaluateReal(cache, 1, &value));
		EXPECT_EQ(1.0, value);
	}
	for (i = 1; i <= size2; ++i)
	{
		Element element = mesh2d.findElementByIdentifier(i);
		EXPECT_EQ(OK, result = cache.setElement(element));
		EXPECT_EQ(OK, result = allFaceField.evaluateReal(cache, 1, &value));
		EXPECT_EQ(1.0, value);
		EXPECT_EQ(OK, result = anyFaceField.evaluateReal(cache, 1, &value));
		EXPECT_EQ((i != 7) ? 1.0 : 0.0, value);
		EXPECT_EQ(OK, result = noFaceField.evaluateReal(cache, 1, &value));
		EXPECT_EQ((i == 7) ? 1.0 : 0.0, value);
	}
	for (i = 1; i <= size1; ++i)
	{
		Element element = mesh1d.findElementByIdentifier(i);
		EXPECT_EQ(OK, result = cache.setElement(element));
		EXPECT_EQ(OK, result = allFaceField.evaluateReal(cache, 1, &value));
		EXPECT_EQ(1.0, value);
		EXPECT_EQ(OK, result = anyFaceField.evaluateReal(cache, 1, &value));
		EXPECT_EQ((i != 16) ? 1.0 : 0.0, value);
		EXPECT_EQ(OK, result = noFaceField.evaluateReal(cache, 1, &value));
		EXPECT_EQ((i == 16) ? 1.0 : 0.0, value);
	}
}

TEST(ZincFieldIsOnFace, invalidArguments)
{
	ZincTestSetupCpp zinc;
	FieldIsOnFace isOnFaceField = zinc.fm.createFieldIsOnFace(Element::FACE_TYPE_INVALID);
	EXPECT_FALSE(isOnFaceField.isValid());
}

TEST(cmzn_field_edge_discontinuity, valid_arguments)
{
	ZincTestSetup zinc;
	int result;

	EXPECT_EQ(CMZN_OK, result = cmzn_region_read_file(zinc.root_region,
		TestResources::getLocation(TestResources::FIELDMODULE_TWO_CUBES_RESOURCE)));

	cmzn_field_id coordinatesField = cmzn_fieldmodule_find_field_by_name(zinc.fm, "coordinates");
	EXPECT_NE((cmzn_field_id)0, coordinatesField);

	cmzn_field_id edgeDiscontinuityField = cmzn_fieldmodule_create_field_edge_discontinuity(
		zinc.fm, coordinatesField);
	EXPECT_NE((cmzn_field_id)0, edgeDiscontinuityField);

	cmzn_field_edge_discontinuity_id edgeDiscontinuity = cmzn_field_cast_edge_discontinuity(edgeDiscontinuityField);
	EXPECT_NE((cmzn_field_edge_discontinuity_id)0, edgeDiscontinuity);

	cmzn_field_edge_discontinuity_measure measure;
	EXPECT_EQ(CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_C1, measure = cmzn_field_edge_discontinuity_get_measure(edgeDiscontinuity));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_edge_discontinuity_set_measure(edgeDiscontinuity, CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_G1));
	EXPECT_EQ(CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_G1, measure = cmzn_field_edge_discontinuity_get_measure(edgeDiscontinuity));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_edge_discontinuity_set_measure(edgeDiscontinuity, CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_SURFACE_NORMAL));
	EXPECT_EQ(CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_SURFACE_NORMAL, measure = cmzn_field_edge_discontinuity_get_measure(edgeDiscontinuity));

	const double one = 1.0;
	cmzn_field_id conditionalField = cmzn_fieldmodule_create_field_constant(zinc.fm, 1, &one);
	EXPECT_NE((cmzn_field_id)0, conditionalField);

	cmzn_field_id tmpField = 0;
	tmpField = cmzn_field_edge_discontinuity_get_conditional_field(edgeDiscontinuity);
	EXPECT_EQ(0, tmpField);
	EXPECT_EQ(CMZN_OK, result = cmzn_field_edge_discontinuity_set_conditional_field(edgeDiscontinuity, conditionalField));
	tmpField = cmzn_field_edge_discontinuity_get_conditional_field(edgeDiscontinuity);
	EXPECT_EQ(conditionalField, tmpField);
	cmzn_field_destroy(&tmpField);
	EXPECT_EQ(CMZN_OK, result = cmzn_field_edge_discontinuity_set_conditional_field(edgeDiscontinuity, 0));
	tmpField = cmzn_field_edge_discontinuity_get_conditional_field(edgeDiscontinuity);
	EXPECT_EQ((cmzn_field_id)0, tmpField);

	cmzn_field_destroy(&conditionalField);
	cmzn_field_destroy(&edgeDiscontinuityField);
	cmzn_field_edge_discontinuity_destroy(&edgeDiscontinuity);
	cmzn_field_destroy(&coordinatesField);
}

TEST(cmzn_field_edge_discontinuity, invalid_arguments)
{
	ZincTestSetup zinc;
	int result;

	const double one = 1.0;
	cmzn_field_id constField = cmzn_fieldmodule_create_field_constant(zinc.fm, 1, &one);
	EXPECT_NE((cmzn_field_id)0, constField);

	cmzn_field_id edgeDiscontinuityField;
	EXPECT_EQ((cmzn_field_id)0, edgeDiscontinuityField = cmzn_fieldmodule_create_field_edge_discontinuity(0, constField));
	EXPECT_EQ((cmzn_field_id)0, edgeDiscontinuityField = cmzn_fieldmodule_create_field_edge_discontinuity(zinc.fm, 0));

	EXPECT_NE((cmzn_field_id)0, edgeDiscontinuityField = cmzn_fieldmodule_create_field_edge_discontinuity(zinc.fm, constField));
	cmzn_field_edge_discontinuity_id edgeDiscontinuity;
	edgeDiscontinuity = cmzn_field_cast_edge_discontinuity(0);
	EXPECT_EQ((cmzn_field_edge_discontinuity_id)0, edgeDiscontinuity);
	edgeDiscontinuity = cmzn_field_cast_edge_discontinuity(edgeDiscontinuityField);
	EXPECT_NE((cmzn_field_edge_discontinuity_id)0, edgeDiscontinuity);

	EXPECT_EQ(CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_INVALID, result = cmzn_field_edge_discontinuity_get_measure(0));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_field_edge_discontinuity_set_measure(0, CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_G1));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_field_edge_discontinuity_set_measure(edgeDiscontinuity, CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_INVALID));
	// can't use MEASURE_SURFACE_NORMAL unless source field has 3 components
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_field_edge_discontinuity_set_measure(edgeDiscontinuity, CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_SURFACE_NORMAL));

	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_field_edge_discontinuity_set_conditional_field(0, constField));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_edge_discontinuity_set_conditional_field(edgeDiscontinuity, constField));
	EXPECT_EQ((cmzn_field_id)0, cmzn_field_edge_discontinuity_get_conditional_field(0));

	cmzn_field_edge_discontinuity_destroy(&edgeDiscontinuity);
	cmzn_field_destroy(&edgeDiscontinuityField);
	cmzn_field_destroy(&constField);
}

TEST(ZincFieldEdgeDiscontinuity, validArguments)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDMODULE_TWO_CUBES_RESOURCE)));

	Field coordinatesField = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinatesField.isValid());

	FieldEdgeDiscontinuity edgeDiscontinuityField = zinc.fm.createFieldEdgeDiscontinuity(coordinatesField);
	EXPECT_TRUE(edgeDiscontinuityField.isValid());

	FieldEdgeDiscontinuity tmpEdgeDiscontinuityField = edgeDiscontinuityField.castEdgeDiscontinuity();
	EXPECT_TRUE(tmpEdgeDiscontinuityField.isValid());

	FieldEdgeDiscontinuity::Measure measure;
	EXPECT_EQ(FieldEdgeDiscontinuity::MEASURE_C1, measure = edgeDiscontinuityField.getMeasure());
	EXPECT_EQ(OK, result = edgeDiscontinuityField.setMeasure(FieldEdgeDiscontinuity::MEASURE_G1));
	EXPECT_EQ(FieldEdgeDiscontinuity::MEASURE_G1, measure = edgeDiscontinuityField.getMeasure());
	EXPECT_EQ(OK, result = edgeDiscontinuityField.setMeasure(FieldEdgeDiscontinuity::MEASURE_SURFACE_NORMAL));
	EXPECT_EQ(FieldEdgeDiscontinuity::MEASURE_SURFACE_NORMAL, measure = edgeDiscontinuityField.getMeasure());

	const double one = 1.0;
	FieldConstant conditionalField = zinc.fm.createFieldConstant(1, &one);
	EXPECT_TRUE(conditionalField.isValid());

	Field tmpField = edgeDiscontinuityField.getConditionalField();
	EXPECT_FALSE(tmpField.isValid());
	EXPECT_EQ(OK, result = edgeDiscontinuityField.setConditionalField(conditionalField));
	tmpField = edgeDiscontinuityField.getConditionalField();
	EXPECT_EQ(conditionalField, tmpField);
	EXPECT_EQ(OK, result = edgeDiscontinuityField.setConditionalField(Field()));
	tmpField = edgeDiscontinuityField.getConditionalField();
	EXPECT_FALSE(tmpField.isValid());
}

TEST(ZincFieldEdgeDiscontinuity, invalidArguments)
{
	ZincTestSetupCpp zinc;
	int result;
	FieldEdgeDiscontinuity edgeDiscontinuityField = zinc.fm.createFieldEdgeDiscontinuity(Field());
	EXPECT_FALSE(edgeDiscontinuityField.isValid());
	FieldEdgeDiscontinuity::Measure measure;
	EXPECT_EQ(FieldEdgeDiscontinuity::MEASURE_INVALID, measure = edgeDiscontinuityField.getMeasure());
	EXPECT_EQ(ERROR_ARGUMENT, result = edgeDiscontinuityField.setMeasure(FieldEdgeDiscontinuity::MEASURE_C1));

	const double one = 1.0;
	FieldConstant constField = zinc.fm.createFieldConstant(1, &one);
	EXPECT_TRUE(constField.isValid());

	edgeDiscontinuityField = zinc.fm.createFieldEdgeDiscontinuity(constField);
	EXPECT_TRUE(edgeDiscontinuityField.isValid());
	EXPECT_EQ(ERROR_ARGUMENT, result = edgeDiscontinuityField.setMeasure(FieldEdgeDiscontinuity::MEASURE_INVALID));
	// can't use MEASURE_SURFACE_NORMAL unless source field has 3 components
	EXPECT_EQ(ERROR_ARGUMENT, result = edgeDiscontinuityField.setMeasure(FieldEdgeDiscontinuity::MEASURE_SURFACE_NORMAL));

	FieldEdgeDiscontinuity tmpEdgeDiscontinuityField = constField.castEdgeDiscontinuity();
	EXPECT_FALSE(tmpEdgeDiscontinuityField.isValid());
}

TEST(ZincFieldEdgeDiscontinuity, evaluate)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDMODULE_ALLSHAPES_RESOURCE)));

	Field coordinatesField = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinatesField.isValid());

	FieldIsExterior isExteriorField = zinc.fm.createFieldIsExterior();
	EXPECT_TRUE(isExteriorField.isValid());

	FieldEdgeDiscontinuity edgeDiscontinuityField = zinc.fm.createFieldEdgeDiscontinuity(coordinatesField);
	EXPECT_TRUE(edgeDiscontinuityField.isValid());
	EXPECT_EQ(OK, result = edgeDiscontinuityField.setConditionalField(isExteriorField));
	EXPECT_EQ(OK, result = edgeDiscontinuityField.setMeasure(FieldEdgeDiscontinuity::MEASURE_C1));

	Mesh mesh1d = zinc.fm.findMeshByDimension(1);
	const int numLines = 33;
	const int numValues = 3*numLines;
	EXPECT_EQ(numLines, mesh1d.getSize());
	double expectedValues_C1[numValues] =
	{
		0, -0, -0,
		-1, 1, 0,
		0, 0, 0,
		0, 0, 0,
		0, 0, 0,
		0, 1, 1,
		0, 1, -1,
		0, -1, 0,
		0, -1, 0,
		0, 0, 0,
		1, 0, 0,
		1, 0, -1,
		-1, 1, -0,
		-1, 1, -1,
		0, 1, 1,
		-1, 1, -1,
		-1, 1, -0,
		0, 0, -1,
		-2, 0, 1,
		1, 0, 1,
		1, -1, -1,
		0, -1, 0,
		0, 0, -1,
		0, -2, 1,
		1, 1, 0,
		1, -1, 0,
		1, 0, 1,
		1, 0, -1,
		0, 1, 1,
		0, 1, -1,
		0, -1, 1,
		-0, -1, -1,
		-1, -2, 2
	};
	double values_C1[numValues];

	Fieldcache cache = zinc.fm.createFieldcache();
	const double xi = 0.5;
	for (int i = 0; i < 33; ++i)
	{
		Element line = mesh1d.findElementByIdentifier(i + 1);
		EXPECT_EQ(OK, result = cache.setMeshLocation(line, 1, &xi));
		EXPECT_EQ(OK, result = edgeDiscontinuityField.evaluateReal(cache, 3, values_C1 + 3*i));
		for (int j = 0; j < 3; ++j)
		 ASSERT_DOUBLE_EQ(expectedValues_C1[3*i + j], values_C1[3*i + j]);
	}

	EXPECT_EQ(OK, result = edgeDiscontinuityField.setMeasure(FieldEdgeDiscontinuity::MEASURE_G1));
	double expectedValues_G1[numValues] =
	{
		0, -0, -0,
		-1, 1, 0,
		0, 0, 0,
		0, 0, 0,
		0, 0, 0,
		0, 1, 1,
		0, 1, -1,
		0.2928932188134525, -0.7071067811865475, 0,
		0.2928932188134525, -0.7071067811865475, 0,
		0, 0, 0,
		0.7071067811865475, 0.2928932188134525, 0,
		0.7071067811865475, 0, -0.7071067811865475,
		0.1055728090000841, 0.4472135954999579, -0,
		-0.1873204098133684, 0.4472135954999579, -0.7071067811865475,
		0, 1, 1,
		-0.7071067811865475, 1, -0.7071067811865475,
		-0.4472135954999579, -0.1055728090000841, -0,
		0, -0.2928932188134525, -0.7071067811865475,
		-1.707106781186548, 0, 0.7071067811865475,
		1, 0, 1,
		1, -0.7071067811865475, -0.7071067811865475,
		-0.2928932188134525, -0.7071067811865475, 0,
		-0.2928932188134525, 0, -0.7071067811865475,
		0, -1.707106781186548, 0.7071067811865475,
		1, 1, 0,
		1, -1, 0,
		1, 0, 1,
		1, 0, -1,
		0, 1, 1,
		0, 1, -1,
		0, -1, 1,
		-0, -1, -1,
		-0.4082482904638631, -1.408248290463863, 0.8164965809277262,
	};
	double values_G1[numValues];
	for (int i = 0; i < 33; ++i)
	{
		Element line = mesh1d.findElementByIdentifier(i + 1);
		EXPECT_EQ(OK, result = cache.setMeshLocation(line, 1, &xi));
		EXPECT_EQ(OK, result = edgeDiscontinuityField.evaluateReal(cache, 3, values_G1 + 3*i));
		for (int j = 0; j < 3; ++j)
			ASSERT_DOUBLE_EQ(expectedValues_G1[3*i + j], values_G1[3*i + j]);
	}

	EXPECT_EQ(OK, result = edgeDiscontinuityField.setMeasure(FieldEdgeDiscontinuity::MEASURE_SURFACE_NORMAL));
	double expectedValues_SN[numValues] =
	{
		0, 0, 0,
		-1, -1, 0,
		0, 0, 0,
		0, 0, 0,
		0, 0, 0,
		0, 1, -1,
		0, -1, -1,
		0, 0, 0,
		0, 0, 0,
		0, 0, 0,
		0, 0, 0,
		0, 0.7071067811865475, -0.2928932188134525,
		0, 0, 0,
		0.7071067811865475, 0, -0.2928932188134525,
		0, -1, 1,
		-0.7071067811865475, -1, -0.7071067811865475,
		0, 0, 0,
		0.1297565119969216, -0.5773502691896258, 0.1297565119969216,
		0.7071067811865475, -0, 1.707106781186548,
		-1, 0, 1,
		-1, -0.7071067811865475, -0.7071067811865475,
		0, 0, 0,
		-0.5773502691896258, 0.1297565119969216, 0.1297565119969216,
		0, -0.7071067811865475, -1.707106781186548,
		-1, 1, 0,
		1, 1, 0,
		1, 0, -1,
		-1, 0, -1,
		0, -1, 1,
		0, 1, 1,
		0, -1, -1,
		0, 1, -1,
		-0.5773502691896258, -0.5773502691896258, -1.577350269189626,
	};
	double values_SN[numValues];
	for (int i = 0; i < 33; ++i)
	{
		Element line = mesh1d.findElementByIdentifier(i + 1);
		EXPECT_EQ(OK, result = cache.setMeshLocation(line, 1, &xi));
		EXPECT_EQ(OK, result = edgeDiscontinuityField.evaluateReal(cache, 3, values_SN + 3*i));
		for (int j = 0; j < 3; ++j)
			ASSERT_DOUBLE_EQ(expectedValues_SN[3*i + j], values_SN[3*i + j]);
	}
}

TEST(ZincFieldEdgeDiscontinuity, invalidEvaluate)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDMODULE_TWO_CUBES_RESOURCE)));

	Field coordinatesField = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinatesField.isValid());

	FieldEdgeDiscontinuity edgeDiscontinuityField = zinc.fm.createFieldEdgeDiscontinuity(coordinatesField);
	EXPECT_TRUE(edgeDiscontinuityField.isValid());

	Fieldcache cache = zinc.fm.createFieldcache();

	// field should not be defined on 2-D elements
	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	EXPECT_TRUE(mesh2d.isValid());
	Element face1 = mesh2d.findElementByIdentifier(1);
	EXPECT_TRUE(face1.isValid());
	double xi2d[2] = { 0.5, 0.5 };
	EXPECT_EQ(OK, result = cache.setMeshLocation(face1, 2, xi2d));
	double outValues[3];
	EXPECT_NE(OK, result = edgeDiscontinuityField.evaluateReal(cache, 3, outValues));
}

// Test changing the basis used in an element
TEST(ZincFieldFiniteElement, redefine_element_field)
{
	ZincTestSetupCpp zinc;
	int result;

	FieldFiniteElement coordinates = zinc.fm.createFieldFiniteElement(/*numberOfComponents*/2);
	EXPECT_TRUE(coordinates.isValid());
	EXPECT_EQ(OK, result = coordinates.setName("coordinates"));
	EXPECT_EQ(OK, result = coordinates.setTypeCoordinate(true));
	EXPECT_EQ(OK, result = coordinates.setManaged(true));
	EXPECT_EQ(OK, result = coordinates.setComponentName(1, "x"));
	EXPECT_EQ(OK, result = coordinates.setComponentName(2, "y"));

	FieldFiniteElement pressure = zinc.fm.createFieldFiniteElement(/*numberOfComponents*/1);
	EXPECT_TRUE(pressure.isValid());
	EXPECT_EQ(OK, result = pressure.setName("pressure"));
	EXPECT_EQ(OK, result = pressure.setManaged(true));

	Nodeset nodeset = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodeset.isValid());

	Nodetemplate nodetemplate = nodeset.createNodetemplate();
	EXPECT_EQ(OK, result = nodetemplate.defineField(coordinates));
	EXPECT_EQ(OK, result = nodetemplate.defineField(pressure));
	Fieldcache fieldcache = zinc.fm.createFieldcache();
	for (int j = 0; j < 3; ++j)
		for (int i = 0; i < 5; ++i)
		{
			int identifier = j*5 + i + 1;
			Node node = nodeset.createNode(identifier, nodetemplate);
			EXPECT_EQ(OK, fieldcache.setNode(node));
			double coordinatesValues[2] = { i*0.5, j*0.5 };
			EXPECT_EQ(OK, coordinates.assignReal(fieldcache, 2, coordinatesValues));
			double pressureValue = (i == 1) || (i == 3) ? 0.0 : 1.0;
			EXPECT_EQ(OK, pressure.assignReal(fieldcache, 1, &pressureValue));
		}

	Mesh mesh = zinc.fm.findMeshByDimension(2);
	EXPECT_TRUE(mesh.isValid());

	Elementbasis bilinearBasis = zinc.fm.createElementbasis(2, Elementbasis::FUNCTION_TYPE_LINEAR_LAGRANGE);
	EXPECT_TRUE(bilinearBasis.isValid());
	Elementbasis biquadraticBasis = zinc.fm.createElementbasis(2, Elementbasis::FUNCTION_TYPE_QUADRATIC_LAGRANGE);
	EXPECT_TRUE(biquadraticBasis.isValid());

	int biquadraticLocalNodeIndexes[9] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	Elementtemplate elementtemplate1 = mesh.createElementtemplate();
	EXPECT_EQ(OK, result = elementtemplate1.setElementShapeType(Element::SHAPE_TYPE_SQUARE));
	EXPECT_EQ(OK, result = elementtemplate1.setNumberOfNodes(9));
	EXPECT_EQ(OK, result = elementtemplate1.defineFieldSimpleNodal(coordinates, /*componentNumber*/-1, biquadraticBasis, 9, biquadraticLocalNodeIndexes));
	EXPECT_EQ(OK, result = elementtemplate1.defineFieldSimpleNodal(pressure, /*componentNumber*/-1, biquadraticBasis, 9, biquadraticLocalNodeIndexes));

	for (int i = 0; i < 2; ++i)
	{
		const int baseNodeIdentifier = 2*i + 1;
		for (int n = 0; n < 9; ++n)
		{
			const int nodeIdentifier = baseNodeIdentifier + (n / 3)*5 + (n % 3);
			Node node = nodeset.findNodeByIdentifier(nodeIdentifier);
			EXPECT_TRUE(node.isValid());
			EXPECT_EQ(OK, result = elementtemplate1.setNode(n + 1, node));
		}
		const int elementIdentifier = i + 1;
		EXPECT_EQ(OK, result = mesh.defineElement(elementIdentifier, elementtemplate1));
	}

	const double xi[2] = { 0.5, 0.5 };
	double coordinateValuesOut[2];
	double pressureValueOut;
	Element element1 = mesh.findElementByIdentifier(1);
	EXPECT_TRUE(element1.isValid());
	Element element2 = mesh.findElementByIdentifier(2);
	EXPECT_TRUE(element2.isValid());

	EXPECT_EQ(OK, result = fieldcache.setMeshLocation(element1, 2, xi));
	EXPECT_EQ(OK, result = coordinates.evaluateReal(fieldcache, 2, coordinateValuesOut));
	EXPECT_DOUBLE_EQ(0.5, coordinateValuesOut[0]);
	EXPECT_DOUBLE_EQ(0.5, coordinateValuesOut[1]);
	EXPECT_EQ(OK, result = pressure.evaluateReal(fieldcache, 1, &pressureValueOut));
	EXPECT_DOUBLE_EQ(0.0, pressureValueOut);
	EXPECT_EQ(OK, result = fieldcache.setMeshLocation(element2, 2, xi));
	EXPECT_EQ(OK, result = coordinates.evaluateReal(fieldcache, 2, coordinateValuesOut));
	EXPECT_DOUBLE_EQ(1.5, coordinateValuesOut[0]);
	EXPECT_DOUBLE_EQ(0.5, coordinateValuesOut[1]);
	EXPECT_EQ(OK, result = pressure.evaluateReal(fieldcache, 1, &pressureValueOut));
	EXPECT_DOUBLE_EQ(0.0, pressureValueOut);

	// redefine element 1 pressure interpolation to be bilinear

	const int bilinearLocalNodeIndexes[4] = { 1, 2, 3, 4 };
	Elementtemplate elementtemplate2 = mesh.createElementtemplate();
	EXPECT_EQ(OK, result = elementtemplate2.setElementShapeType(Element::SHAPE_TYPE_INVALID));
	EXPECT_EQ(OK, result = elementtemplate2.setNumberOfNodes(4));
	EXPECT_EQ(OK, result = elementtemplate2.defineFieldSimpleNodal(pressure, /*componentNumber*/-1, bilinearBasis, 4, bilinearLocalNodeIndexes));

	EXPECT_EQ(OK, result = elementtemplate2.setNode(1, nodeset.findNodeByIdentifier(1)));
	EXPECT_EQ(OK, result = elementtemplate2.setNode(2, nodeset.findNodeByIdentifier(3)));
	EXPECT_EQ(OK, result = elementtemplate2.setNode(3, nodeset.findNodeByIdentifier(11)));
	EXPECT_EQ(OK, result = elementtemplate2.setNode(4, nodeset.findNodeByIdentifier(13)));
	EXPECT_EQ(OK, result = element1.merge(elementtemplate2));

	EXPECT_EQ(OK, result = fieldcache.setMeshLocation(element1, 2, xi));
	EXPECT_EQ(OK, result = coordinates.evaluateReal(fieldcache, 2, coordinateValuesOut));
	EXPECT_DOUBLE_EQ(0.5, coordinateValuesOut[0]);
	EXPECT_DOUBLE_EQ(0.5, coordinateValuesOut[1]);
	EXPECT_EQ(OK, result = pressure.evaluateReal(fieldcache, 1, &pressureValueOut));
	EXPECT_DOUBLE_EQ(1.0, pressureValueOut);
	EXPECT_EQ(OK, result = fieldcache.setMeshLocation(element2, 2, xi));
	EXPECT_EQ(OK, result = coordinates.evaluateReal(fieldcache, 2, coordinateValuesOut));
	EXPECT_DOUBLE_EQ(1.5, coordinateValuesOut[0]);
	EXPECT_DOUBLE_EQ(0.5, coordinateValuesOut[1]);
	EXPECT_EQ(OK, result = pressure.evaluateReal(fieldcache, 1, &pressureValueOut));
	EXPECT_DOUBLE_EQ(0.0, pressureValueOut);

	// redefine element 2 pressure interpolation to be bilinear
	// this time test between begin/end change

	zinc.fm.beginChange();
	EXPECT_EQ(OK, result = elementtemplate2.setNode(1, nodeset.findNodeByIdentifier(3)));
	EXPECT_EQ(OK, result = elementtemplate2.setNode(2, nodeset.findNodeByIdentifier(5)));
	EXPECT_EQ(OK, result = elementtemplate2.setNode(3, nodeset.findNodeByIdentifier(13)));
	EXPECT_EQ(OK, result = elementtemplate2.setNode(4, nodeset.findNodeByIdentifier(15)));
	EXPECT_EQ(OK, result = element2.merge(elementtemplate2));

	EXPECT_EQ(OK, result = fieldcache.setMeshLocation(element1, 2, xi));
	EXPECT_EQ(OK, result = pressure.evaluateReal(fieldcache, 1, &pressureValueOut));
	EXPECT_DOUBLE_EQ(1.0, pressureValueOut);
	EXPECT_EQ(OK, result = fieldcache.setMeshLocation(element2, 2, xi));
	EXPECT_EQ(OK, result = pressure.evaluateReal(fieldcache, 1, &pressureValueOut));
	EXPECT_DOUBLE_EQ(1.0, pressureValueOut);
	zinc.fm.endChange();
}


TEST(ZincFieldNodeLookup, Evaluate)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDMODULE_TWO_CUBES_RESOURCE)));

	Field coordinatesField = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinatesField.isValid());

	Nodeset nodeset = zinc.fm.findNodesetByName("nodes");
	EXPECT_TRUE(nodeset.isValid());

	Node node = nodeset.findNodeByIdentifier(12);
	EXPECT_TRUE(node.isValid());

	FieldNodeLookup nodeLookup = zinc.fm.createFieldNodeLookup(coordinatesField, node);
	EXPECT_TRUE(nodeLookup.isValid());

	Fieldcache cache = zinc.fm.createFieldcache();
	EXPECT_TRUE(cache.isValid());

	double values[3] = {0.0, 0.0, 0.0};
	EXPECT_EQ(OK, result = nodeLookup.evaluateReal(cache, 3, &values[0]));
	EXPECT_DOUBLE_EQ(20.0, values[0]);
	EXPECT_DOUBLE_EQ(10.0, values[1]);
	EXPECT_DOUBLE_EQ(10.0, values[2]);
}


class FieldmodulecallbackCheckExterior : public Fieldmodulecallback
{
public:
	Fieldcache& cache;
	FieldIsExterior& isExteriorField;
	Fieldmoduleevent lastEvent;

	FieldmodulecallbackCheckExterior(Fieldcache& cacheIn, FieldIsExterior& isExteriorFieldIn) :
		cache(cacheIn),
		isExteriorField(isExteriorFieldIn)
	{ }

	virtual void operator()(const Fieldmoduleevent &event)
	{
		double isExteriorValue;
		EXPECT_EQ(OK, this->isExteriorField.evaluateReal(this->cache, 1, &isExteriorValue));
		EXPECT_DOUBLE_EQ(1.0, isExteriorValue);
		this->lastEvent = event;
	}
};

// Test merging a mesh in does not leave faces with temporary elements as
// additional parents, which stops the 'is exterior' logic from working.
TEST(ZincFieldFiniteElement, issue_3892_temporary_element_parents)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE)));
	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	EXPECT_TRUE(mesh2d.isValid());
	FieldIsExterior isExteriorField = zinc.fm.createFieldIsExterior();
	EXPECT_TRUE(isExteriorField.isValid());

	Element face1 = mesh2d.findElementByIdentifier(1);
	EXPECT_TRUE(face1.isValid());
	Fieldcache cache = zinc.fm.createFieldcache();
	EXPECT_TRUE(cache.isValid());
	cache.setElement(face1);
	double isExteriorValue;
	EXPECT_EQ(OK, result = isExteriorField.evaluateReal(cache, 1, &isExteriorValue));
	EXPECT_DOUBLE_EQ(1.0, isExteriorValue);

	Fieldmodulenotifier notifier = zinc.fm.createFieldmodulenotifier();
	EXPECT_TRUE(notifier.isValid());
	FieldmodulecallbackCheckExterior checkExteriorCallback(cache, isExteriorField);
	EXPECT_EQ(CMZN_OK, result = notifier.setCallback(checkExteriorCallback));

	EXPECT_EQ(OK, result = zinc.root_region.readFile(TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE)));
	EXPECT_TRUE(checkExteriorCallback.lastEvent.isValid());
	EXPECT_EQ(OK, result = isExteriorField.evaluateReal(cache, 1, &isExteriorValue));
	EXPECT_DOUBLE_EQ(1.0, isExteriorValue);
}

TEST(ZincFieldFiniteElement, get_setNodeParameters)
{
	ZincTestSetupCpp zinc;

	FieldFiniteElement feField = zinc.fm.createFieldFiniteElement(3);
	EXPECT_TRUE(feField.isValid());

	Nodeset nodeset = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodeset.isValid());
	Nodetemplate nodetemplate = nodeset.createNodetemplate();
	EXPECT_EQ(OK, nodetemplate.defineField(feField));
	EXPECT_EQ(OK, nodetemplate.setValueNumberOfVersions(feField, -1, Node::VALUE_LABEL_VALUE, 2));
	EXPECT_EQ(2, nodetemplate.getValueNumberOfVersions(feField, -1, Node::VALUE_LABEL_VALUE));
	EXPECT_EQ(OK, nodetemplate.setValueNumberOfVersions(feField, 2, Node::VALUE_LABEL_VALUE, 3));
	EXPECT_EQ(OK, nodetemplate.setValueNumberOfVersions(feField, -1, Node::VALUE_LABEL_D_DS1, 1));
	EXPECT_EQ(OK, nodetemplate.setValueNumberOfVersions(feField, 1, Node::VALUE_LABEL_D_DS1, 2));

	EXPECT_EQ(2, nodetemplate.getValueNumberOfVersions(feField, 1, Node::VALUE_LABEL_VALUE));
	EXPECT_EQ(3, nodetemplate.getValueNumberOfVersions(feField, 2, Node::VALUE_LABEL_VALUE));
	EXPECT_EQ(2, nodetemplate.getValueNumberOfVersions(feField, 3, Node::VALUE_LABEL_VALUE));
	EXPECT_EQ(-1, nodetemplate.getValueNumberOfVersions(feField, -1, Node::VALUE_LABEL_VALUE));

	// note prior restriction to same number of versions for all derivatives / value labels has been removed
	// hence get the following get as many versions as for VALUE
	EXPECT_EQ(2, nodetemplate.getValueNumberOfVersions(feField, 1, Node::VALUE_LABEL_D_DS1));
	EXPECT_EQ(1, nodetemplate.getValueNumberOfVersions(feField, 2, Node::VALUE_LABEL_D_DS1));
	EXPECT_EQ(1, nodetemplate.getValueNumberOfVersions(feField, 3, Node::VALUE_LABEL_D_DS1));
	EXPECT_EQ(-1, nodetemplate.getValueNumberOfVersions(feField, -1, Node::VALUE_LABEL_D_DS1));

	Fieldcache cache = zinc.fm.createFieldcache();
	EXPECT_TRUE(cache.isValid());

	Node node = nodeset.createNode(-1, nodetemplate);
	EXPECT_TRUE(node.isValid());
	EXPECT_EQ(OK, cache.setNode(node));

	// set/get all components

	const double valuesIn1[3] = { 2, 3, 4 };
	double valuesOut1[3];
	const double valuesIn2[3] = { 1, 6, 5 };
	double valuesOut2[3];
	const double valuesIn3[3] = { 7, 8, 9 };
	double valuesOut3[3];
	const double derivativesIn1[3] = { 2.1, 3.2, 4.3 };
	double derivativesOut1[3];
	const double derivativesIn2[3] = { 1.0, 6.7, 5.8 };
	double derivativesOut2[3];
	EXPECT_EQ(OK, feField.setNodeParameters(cache, -1, Node::VALUE_LABEL_VALUE, 1, 3, valuesIn1));
	EXPECT_EQ(OK, feField.setNodeParameters(cache, -1, Node::VALUE_LABEL_VALUE, 2, 3, valuesIn2));
	EXPECT_EQ(WARNING_PART_DONE, feField.setNodeParameters(cache, -1, Node::VALUE_LABEL_VALUE, 3, 3, valuesIn3));
	EXPECT_EQ(ERROR_NOT_FOUND, feField.setNodeParameters(cache, -1, Node::VALUE_LABEL_VALUE, 4, 3, valuesIn3));
	EXPECT_EQ(OK, feField.setNodeParameters(cache, -1, Node::VALUE_LABEL_D_DS1, 1, 3, derivativesIn1));
	EXPECT_EQ(WARNING_PART_DONE, feField.setNodeParameters(cache, -1, Node::VALUE_LABEL_D_DS1, 2, 3, derivativesIn2));

	EXPECT_EQ(OK, feField.getNodeParameters(cache, -1, Node::VALUE_LABEL_VALUE, 1, 3, valuesOut1));
	EXPECT_EQ(OK, feField.getNodeParameters(cache, -1, Node::VALUE_LABEL_VALUE, 2, 3, valuesOut2));
	EXPECT_EQ(WARNING_PART_DONE, feField.getNodeParameters(cache, -1, Node::VALUE_LABEL_VALUE, 3, 3, valuesOut3));
	EXPECT_EQ(OK, feField.getNodeParameters(cache, -1, Node::VALUE_LABEL_D_DS1, 1, 3, derivativesOut1));
	EXPECT_EQ(WARNING_PART_DONE, feField.getNodeParameters(cache, -1, Node::VALUE_LABEL_D_DS1, 2, 3, derivativesOut2));
	EXPECT_DOUBLE_EQ(valuesIn1[0], valuesOut1[0]);
	EXPECT_DOUBLE_EQ(valuesIn1[1], valuesOut1[1]);
	EXPECT_DOUBLE_EQ(valuesIn1[2], valuesOut1[2]);
	EXPECT_DOUBLE_EQ(valuesIn2[0], valuesOut2[0]);
	EXPECT_DOUBLE_EQ(valuesIn2[1], valuesOut2[1]);
	EXPECT_DOUBLE_EQ(valuesIn2[2], valuesOut2[2]);
	EXPECT_DOUBLE_EQ(0.0, valuesOut3[0]);
	EXPECT_DOUBLE_EQ(valuesIn3[1], valuesOut3[1]);
	EXPECT_DOUBLE_EQ(0.0, valuesOut3[2]);
	EXPECT_DOUBLE_EQ(derivativesIn1[0], derivativesOut1[0]);
	EXPECT_DOUBLE_EQ(derivativesIn1[1], derivativesOut1[1]);
	EXPECT_DOUBLE_EQ(derivativesIn1[2], derivativesOut1[2]);
	EXPECT_DOUBLE_EQ(derivativesIn2[0], derivativesOut2[0]);
	EXPECT_DOUBLE_EQ(0.0, derivativesOut2[1]);
	EXPECT_DOUBLE_EQ(0.0, derivativesOut2[2]);

	// set/get individual components

	const double valueIn1 = 2.34;
	double valueOut1;
	const double valueIn2 = 3.45;
	double valueOut2;
	double dummyOut;
	const double valueIn3 = 4.56;
	double valueOut3;
	EXPECT_EQ(OK, feField.setNodeParameters(cache, 1, Node::VALUE_LABEL_VALUE, 1, 1, &valueIn1));
	EXPECT_EQ(OK, feField.getNodeParameters(cache, 1, Node::VALUE_LABEL_VALUE, 1, 1, &valueOut1));
	EXPECT_DOUBLE_EQ(valueIn1, valueOut1);
	EXPECT_EQ(OK, feField.setNodeParameters(cache, 2, Node::VALUE_LABEL_VALUE, 1, 1, &valueIn1));
	EXPECT_EQ(OK, feField.getNodeParameters(cache, 2, Node::VALUE_LABEL_VALUE, 1, 1, &valueOut1));
	EXPECT_DOUBLE_EQ(valueIn1, valueOut1);
	EXPECT_EQ(OK, feField.setNodeParameters(cache, 3, Node::VALUE_LABEL_VALUE, 1, 1, &valueIn1));
	EXPECT_EQ(OK, feField.getNodeParameters(cache, 3, Node::VALUE_LABEL_VALUE, 1, 1, &valueOut1));
	EXPECT_DOUBLE_EQ(valueIn1, valueOut1);
	// test invalid versions:
	EXPECT_EQ(ERROR_NOT_FOUND, feField.setNodeParameters(cache, 1, Node::VALUE_LABEL_VALUE, 3, 1, &valueIn1));
	EXPECT_EQ(ERROR_NOT_FOUND, feField.getNodeParameters(cache, 1, Node::VALUE_LABEL_VALUE, 3, 1, &dummyOut));
	EXPECT_EQ(ERROR_NOT_FOUND, feField.setNodeParameters(cache, 2, Node::VALUE_LABEL_D_DS1, 2, 1, &valueIn1));
	EXPECT_EQ(ERROR_NOT_FOUND, feField.getNodeParameters(cache, 2, Node::VALUE_LABEL_D_DS1, 2, 1, &dummyOut));
	EXPECT_EQ(OK, feField.setNodeParameters(cache, 2, Node::VALUE_LABEL_VALUE, 3, 1, &valueIn2));
	EXPECT_EQ(OK, feField.getNodeParameters(cache, 2, Node::VALUE_LABEL_VALUE, 3, 1, &valueOut2));
	EXPECT_DOUBLE_EQ(valueIn2, valueOut2);
	EXPECT_EQ(OK, feField.setNodeParameters(cache, 1, Node::VALUE_LABEL_D_DS1, 2, 1, &valueIn3));
	EXPECT_EQ(OK, feField.getNodeParameters(cache, 1, Node::VALUE_LABEL_D_DS1, 2, 1, &valueOut3));
	EXPECT_DOUBLE_EQ(valueIn3, valueOut3);
}


TEST(ZincNodetemplate, define_undefineField)
{
	ZincTestSetupCpp zinc;

	FieldFiniteElement feField = zinc.fm.createFieldFiniteElement(1);
	EXPECT_TRUE(feField.isValid());

	Nodeset nodeset = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodeset.isValid());

	Nodetemplate nodetemplate1 = nodeset.createNodetemplate();
	EXPECT_EQ(ERROR_ARGUMENT, nodetemplate1.defineField(Field()));
	EXPECT_EQ(OK, nodetemplate1.defineField(feField));

	Nodetemplate nodetemplate2 = nodeset.createNodetemplate();
	EXPECT_EQ(OK, nodetemplate2.defineField(feField));
	EXPECT_EQ(OK, nodetemplate2.setValueNumberOfVersions(feField, -1, Node::VALUE_LABEL_VALUE, 2));

	Nodetemplate nodetemplate3 = nodeset.createNodetemplate();
	EXPECT_EQ(ERROR_ARGUMENT, nodetemplate3.undefineField(Field()));
	EXPECT_EQ(OK, nodetemplate3.undefineField(feField));

	Nodetemplate nodetemplate4 = nodeset.createNodetemplate();
	EXPECT_EQ(OK, nodetemplate4.defineField(feField));
	EXPECT_EQ(ERROR_ARGUMENT, nodetemplate4.removeField(Field()));
	EXPECT_EQ(OK, nodetemplate4.removeField(feField));

	Node node1 = nodeset.createNode(-1, nodetemplate1);
	EXPECT_TRUE(node1.isValid());
	Node node2 = nodeset.createNode(-1, nodetemplate2);
	EXPECT_TRUE(node2.isValid());
	Node node3 = nodeset.createNode(-1, nodetemplate3);
	EXPECT_TRUE(node3.isValid());
	Node node4 = nodeset.createNode(-1, nodetemplate4);
	EXPECT_TRUE(node4.isValid());

	Nodetemplate nodetemplate5 = nodeset.createNodetemplate();
	EXPECT_EQ(ERROR_ARGUMENT, nodetemplate5.defineFieldFromNode(Field(), node1));
	EXPECT_EQ(ERROR_ARGUMENT, nodetemplate5.defineFieldFromNode(feField, Node()));
	EXPECT_EQ(OK, nodetemplate5.defineFieldFromNode(feField, node1));
	EXPECT_EQ(1, nodetemplate5.getValueNumberOfVersions(feField, -1, Node::VALUE_LABEL_VALUE));
	EXPECT_EQ(OK, nodetemplate5.defineFieldFromNode(feField, node2));
	EXPECT_EQ(2, nodetemplate5.getValueNumberOfVersions(feField, -1, Node::VALUE_LABEL_VALUE));
	EXPECT_EQ(ERROR_NOT_FOUND, nodetemplate5.defineFieldFromNode(feField, node3));
	EXPECT_EQ(ERROR_NOT_FOUND, nodetemplate5.setValueNumberOfVersions(feField, -1, Node::VALUE_LABEL_VALUE, 1));

	EXPECT_EQ(OK, node1.merge(nodetemplate3));
	EXPECT_EQ(ERROR_NOT_FOUND, nodetemplate5.defineFieldFromNode(feField, node1));
	EXPECT_EQ(ERROR_NOT_FOUND, nodetemplate5.setValueNumberOfVersions(feField, -1, Node::VALUE_LABEL_VALUE, 1));
	EXPECT_EQ(OK, node3.merge(nodetemplate2));
	EXPECT_EQ(OK, nodetemplate5.defineFieldFromNode(feField, node3));
	EXPECT_EQ(2, nodetemplate5.getValueNumberOfVersions(feField, -1, Node::VALUE_LABEL_VALUE));

	EXPECT_EQ(OK, nodetemplate5.undefineField(feField));
	EXPECT_EQ(OK, node2.merge(nodetemplate5));
	EXPECT_EQ(ERROR_NOT_FOUND, nodetemplate5.defineFieldFromNode(feField, node2));
	EXPECT_EQ(ERROR_NOT_FOUND, nodetemplate5.setValueNumberOfVersions(feField, -1, Node::VALUE_LABEL_VALUE, 1));
	EXPECT_EQ(OK, nodetemplate5.defineFieldFromNode(feField, node3));
	EXPECT_EQ(2, nodetemplate5.getValueNumberOfVersions(feField, -1, Node::VALUE_LABEL_VALUE));
	EXPECT_EQ(ERROR_NOT_FOUND, nodetemplate5.defineFieldFromNode(feField, node4));
	EXPECT_EQ(ERROR_NOT_FOUND, nodetemplate5.setValueNumberOfVersions(feField, -1, Node::VALUE_LABEL_VALUE, 1));
}

// Issue 50: Find mesh location was caching wrong xi between fieldmodule begin/end change
// Also, convergence was difficult for far away points with high curvature elements.
// This test loads a curved heart surface mesh and finds nearest xi to 4 quite distant points.
TEST(ZincFieldFindMeshLocation, issue_50_find_xi_cache_and_convergence)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(TestResources::getLocation(TestResources::FIELDMODULE_HEART_SURFACE_RESOURCE)));
	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	EXPECT_TRUE(mesh2d.isValid());
	EXPECT_EQ(OK, result = zinc.root_region.readFile(TestResources::getLocation(TestResources::FIELDMODULE_PLATE_600X300_RESOURCE)));
	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());

	FieldDerivative d1 = zinc.fm.createFieldDerivative(coordinates, 1);
	EXPECT_TRUE(d1.isValid());
	FieldDerivative d2 = zinc.fm.createFieldDerivative(coordinates, 2);
	EXPECT_TRUE(d2.isValid());
	FieldCrossProduct inside = zinc.fm.createFieldCrossProduct(d1, d2);
	EXPECT_TRUE(inside.isValid());
	FieldNormalise norm_inside = zinc.fm.createFieldNormalise(inside);
	EXPECT_TRUE(norm_inside.isValid());

	Field plate_coordinates = zinc.fm.findFieldByName("plate_coordinates");
	EXPECT_TRUE(plate_coordinates.isValid());
	Mesh heart_mesh = zinc.fm.findMeshByName("heart.mesh2d");
	EXPECT_TRUE(heart_mesh.isValid());
	FieldFindMeshLocation find_heart = zinc.fm.createFieldFindMeshLocation(plate_coordinates, coordinates, heart_mesh);
	EXPECT_TRUE(find_heart.isValid());
	EXPECT_EQ(RESULT_OK, find_heart.setSearchMode(FieldFindMeshLocation::SEARCH_MODE_NEAREST));
	FieldEmbedded find_heart_coordinates = zinc.fm.createFieldEmbedded(coordinates, find_heart);
	EXPECT_TRUE(find_heart_coordinates.isValid());
	Field find_heart_vector = find_heart_coordinates - plate_coordinates;
	EXPECT_TRUE(find_heart_vector.isValid());
	FieldNormalise find_heart_norm_vector = zinc.fm.createFieldNormalise(find_heart_vector);
	EXPECT_TRUE(find_heart_norm_vector.isValid());
	FieldEmbedded find_heart_norm_inside = zinc.fm.createFieldEmbedded(norm_inside, find_heart);
	EXPECT_TRUE(find_heart_norm_inside.isValid());
	FieldDotProduct find_heart_dot = zinc.fm.createFieldDotProduct(find_heart_norm_vector, find_heart_norm_inside);
	EXPECT_TRUE(find_heart_dot.isValid());

	// between begin/endChange, field evaluation value caching is turned off meaning
	// find_mesh_location is done twice. This brought up its incorrect cache behaviour.
	zinc.fm.beginChange();

	Fieldcache cache = zinc.fm.createFieldcache();
	Element plate_element = mesh2d.findElementByIdentifier(10001);
	EXPECT_TRUE(plate_element.isValid());

	// these are fairly coarse tolerances as full convergence is not expected
	// with current 50 iteration limit for this test case
	const double xiTol = 1.0E-4;
	const double xTol = 0.1;
	const double xi[4][2] = {
		{ 0.25, 0.25 },
		{ 0.75, 0.25 },
		{ 0.25, 0.75 },
		{ 0.75, 0.75 }
	};
	double xi_out[2];
	const double expected_xi_out[4][2] = {
		{0.12406714819971967, 0.25562651372669704},
		{ 0.88236601025702055, 0.21582949398565682 },
		{ 0.48618917733895772, 0.10260652064158104 },
		{ 0.96903006205426667, 0.77350111489974804 }
	};
	Element element_out;
	const int expected_element_out_identifier[4] = { 17, 14, 22, 13 };
	double x_out[3];
	const double expected_x_out[4][3] = {
		{ -40.908162481673017, -40.601121600661465, 362.32017916586187 },
		{90.298764787257539, -57.849226522130387, 337.95645572195986},
		{-36.346997111262070, -11.216744873090230, 380.68037451384760},
		{68.763718878371719, 2.1624108948975049, 351.46269549335761}
	};
	double vector_out[3];
	const double expected_vector_out[4][3] = {
		{ 109.09183751832698, 34.398878399338535, 12.320179165861873 },
		{ -59.701235212742461, 17.150773477869613, -12.043544278040144 },
		{ 113.65300288873793, -86.216744873090235, 30.680374513847596 },
		{ -81.236281121628281, -72.837589105102495, 1.4626954933576144 }
	};
	double dot;
	double dx_out[3];
	for (int xin = 0; xin < 4; ++xin)
	{
		EXPECT_EQ(RESULT_OK, cache.setMeshLocation(plate_element, 2, xi[xin]));

		Element element_out = find_heart.evaluateMeshLocation(cache, 2, xi_out);
		EXPECT_TRUE(element_out.isValid());
		int element_out_number = element_out.getIdentifier();
		EXPECT_EQ(expected_element_out_identifier[xin], element_out_number);
		EXPECT_NEAR(expected_xi_out[xin][0], xi_out[0], xiTol);
		EXPECT_NEAR(expected_xi_out[xin][1], xi_out[1], xiTol);

		element_out = find_heart.evaluateMeshLocation(cache, 2, xi_out);
		EXPECT_TRUE(element_out.isValid());
		element_out_number = element_out.getIdentifier();
		EXPECT_EQ(expected_element_out_identifier[xin], element_out_number);
		EXPECT_NEAR(expected_xi_out[xin][0], xi_out[0], xiTol);
		EXPECT_NEAR(expected_xi_out[xin][1], xi_out[1], xiTol);

		EXPECT_EQ(RESULT_OK, find_heart_coordinates.evaluateReal(cache, 3, x_out));
		EXPECT_NEAR(expected_x_out[xin][0], x_out[0], xTol);
		EXPECT_NEAR(expected_x_out[xin][1], x_out[1], xTol);
		EXPECT_NEAR(expected_x_out[xin][2], x_out[2], xTol);

		EXPECT_EQ(RESULT_OK, find_heart_vector.evaluateReal(cache, 3, vector_out));
		EXPECT_NEAR(expected_vector_out[xin][0], vector_out[0], xTol);
		EXPECT_NEAR(expected_vector_out[xin][1], vector_out[1], xTol);
		EXPECT_NEAR(expected_vector_out[xin][2], vector_out[2], xTol);

		// all locations are outside of heart surface. Since heart is smooth, dot product
		// of projection should be normal to surface and same direction as inside surface
		// normal. The following confirms this (but needs a loose tolerance).
		EXPECT_EQ(RESULT_OK, find_heart_dot.evaluateReal(cache, 1, &dot));
		EXPECT_GT(dot, 0.995);

		EXPECT_EQ(RESULT_OK, cache.setMeshLocation(element_out, 2, xi_out));
		EXPECT_EQ(RESULT_OK, coordinates.evaluateReal(cache, 3, dx_out));
		EXPECT_NEAR(expected_x_out[xin][0], dx_out[0], xTol);
		EXPECT_NEAR(expected_x_out[xin][1], dx_out[1], xTol);
		EXPECT_NEAR(expected_x_out[xin][2], dx_out[2], xTol);
	}

	zinc.fm.endChange();
}

TEST(ZincElementfieldtemplate, element_based_constant)
{
	ZincTestSetupCpp zinc;

	Mesh mesh1d = zinc.fm.findMeshByDimension(1);
	EXPECT_TRUE(mesh1d.isValid());

	Elementbasis constantBasis = zinc.fm.createElementbasis(1, Elementbasis::FUNCTION_TYPE_CONSTANT);
	EXPECT_TRUE(constantBasis.isValid());
	Elementfieldtemplate eft = mesh1d.createElementfieldtemplate(constantBasis);
	EXPECT_TRUE(eft.isValid());

	EXPECT_EQ(Elementfieldtemplate::PARAMETER_MAPPING_MODE_NODE, eft.getParameterMappingMode());
	EXPECT_EQ(1, eft.getNumberOfFunctions());
	EXPECT_EQ(1, eft.getNumberOfLocalNodes());
	EXPECT_EQ(0, eft.getNumberOfLocalScaleFactors());
	EXPECT_EQ(1, eft.getFunctionNumberOfTerms(1));

	EXPECT_EQ(RESULT_OK, eft.setParameterMappingMode(Elementfieldtemplate::PARAMETER_MAPPING_MODE_ELEMENT));
	EXPECT_EQ(Elementfieldtemplate::PARAMETER_MAPPING_MODE_ELEMENT, eft.getParameterMappingMode());

	// many APIs are not supported for element-based parameter map:
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, eft.setNumberOfLocalNodes(1));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, eft.setNumberOfLocalScaleFactors(1));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, eft.setFunctionNumberOfTerms(1, 2));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, eft.setFunctionNumberOfTerms(1, 0));
}

TEST(ZincElementfieldtemplate, node_based_bilinear)
{
	ZincTestSetupCpp zinc;

	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	EXPECT_TRUE(mesh2d.isValid());

	// test invalid basis for mesh dimension
	Elementbasis linearBasis = zinc.fm.createElementbasis(1, Elementbasis::FUNCTION_TYPE_LINEAR_LAGRANGE);
	EXPECT_TRUE(linearBasis.isValid());
	Elementfieldtemplate eft = mesh2d.createElementfieldtemplate(linearBasis);
	EXPECT_FALSE(eft.isValid());

	// test default node interpolation
	Elementbasis bilinearBasis = zinc.fm.createElementbasis(2, Elementbasis::FUNCTION_TYPE_LINEAR_LAGRANGE);
	EXPECT_TRUE(bilinearBasis.isValid());
	eft = mesh2d.createElementfieldtemplate(bilinearBasis);
	EXPECT_TRUE(eft.isValid());
	EXPECT_EQ(Elementfieldtemplate::PARAMETER_MAPPING_MODE_NODE, eft.getParameterMappingMode());

	EXPECT_EQ(4, eft.getNumberOfFunctions());
	EXPECT_EQ(4, eft.getNumberOfLocalNodes());
	EXPECT_EQ(0, eft.getNumberOfLocalScaleFactors());
	for (int i = 1; i <= 4; ++i)
	{
		EXPECT_EQ(1, eft.getFunctionNumberOfTerms(i));
		EXPECT_EQ(i, eft.getTermLocalNodeIndex(/*functionNumber*/i, /*term*/1));
		EXPECT_EQ(Node::VALUE_LABEL_VALUE, eft.getTermNodeValueLabel(/*functionNumber*/i, /*term*/1));
		EXPECT_EQ(1, eft.getTermNodeVersion(/*functionNumber*/i, /*term*/1));
	}
	// invalid arguments:
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, eft.getFunctionNumberOfTerms(0));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, eft.getFunctionNumberOfTerms(5));
	EXPECT_EQ(0, eft.getTermLocalNodeIndex(/*functionNumber*/1, /*term*/0));
	EXPECT_EQ(0, eft.getTermLocalNodeIndex(/*functionNumber*/1, /*term*/2));
	EXPECT_EQ(Node::VALUE_LABEL_INVALID, eft.getTermNodeValueLabel(/*functionNumber*/1, /*term*/0));
	EXPECT_EQ(Node::VALUE_LABEL_INVALID, eft.getTermNodeValueLabel(/*functionNumber*/1, /*term*/2));
	EXPECT_EQ(0, eft.getTermNodeVersion(/*functionNumber*/1, /*term*/0));
	EXPECT_EQ(0, eft.getTermNodeVersion(/*functionNumber*/1, /*term*/2));
	EXPECT_EQ(-1, eft.getTermScaling(/*functionNumber*/1, /*term*/1, 0, 0)); // invalid with no local scale factors
	EXPECT_EQ(Elementfieldtemplate::SCALE_FACTOR_TYPE_INVALID, eft.getScaleFactorType(1));
	EXPECT_EQ(-1, eft.getScaleFactorIdentifier(1));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, eft.setNumberOfLocalScaleFactors(-1));

	// test modifications - number and type/version of local scale factors
	EXPECT_EQ(RESULT_OK, eft.setNumberOfLocalScaleFactors(3));
	EXPECT_EQ(3, eft.getNumberOfLocalScaleFactors());
	for (int sf = 1; sf <= 3; ++sf)
	{
		EXPECT_EQ(Elementfieldtemplate::SCALE_FACTOR_TYPE_ELEMENT_GENERAL, eft.getScaleFactorType(sf));
		EXPECT_EQ(RESULT_OK, eft.setScaleFactorType(sf, Elementfieldtemplate::SCALE_FACTOR_TYPE_NODE_PATCH));
		EXPECT_EQ(Elementfieldtemplate::SCALE_FACTOR_TYPE_NODE_PATCH, eft.getScaleFactorType(sf));
		EXPECT_EQ(0, eft.getScaleFactorIdentifier(sf));
		EXPECT_EQ(RESULT_OK, eft.setScaleFactorIdentifier(sf, 2));
		EXPECT_EQ(2, eft.getScaleFactorIdentifier(sf));
	}
	EXPECT_EQ(RESULT_OK, eft.setNumberOfLocalScaleFactors(5));
	EXPECT_EQ(5, eft.getNumberOfLocalScaleFactors());
	for (int sf = 1; sf <= 3; ++sf)
	{
		EXPECT_EQ(Elementfieldtemplate::SCALE_FACTOR_TYPE_NODE_PATCH, eft.getScaleFactorType(sf));
		EXPECT_EQ(2, eft.getScaleFactorIdentifier(sf));
	}
	for (int sf = 4; sf <= 5; ++sf)
	{
		EXPECT_EQ(Elementfieldtemplate::SCALE_FACTOR_TYPE_ELEMENT_GENERAL, eft.getScaleFactorType(sf));
		EXPECT_EQ(0, eft.getScaleFactorIdentifier(sf));
	}
	EXPECT_EQ(RESULT_OK, eft.setNumberOfLocalScaleFactors(4));
	EXPECT_EQ(4, eft.getNumberOfLocalScaleFactors());
	// invalid arguments:
	EXPECT_EQ(Elementfieldtemplate::SCALE_FACTOR_TYPE_INVALID, eft.getScaleFactorType(0));
	EXPECT_EQ(Elementfieldtemplate::SCALE_FACTOR_TYPE_INVALID, eft.getScaleFactorType(5));
	EXPECT_EQ(-1, eft.getScaleFactorIdentifier(0));
	EXPECT_EQ(-1, eft.getScaleFactorIdentifier(5));

	// test modifications - number of local nodes
	EXPECT_EQ(RESULT_OK, eft.setNumberOfLocalNodes(5));
	EXPECT_EQ(5, eft.getNumberOfLocalNodes());
	EXPECT_EQ(RESULT_OK, eft.setNumberOfLocalNodes(4));
	EXPECT_EQ(4, eft.getNumberOfLocalNodes());
	// invalid arguments:
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, eft.setNumberOfLocalNodes(0));
	EXPECT_EQ(4, eft.getNumberOfLocalNodes());

	// test modifications - number of terms
	EXPECT_EQ(RESULT_OK, eft.setFunctionNumberOfTerms(/*functionNumber*/1, 2));
	EXPECT_EQ(2, eft.getFunctionNumberOfTerms(/*functionNumber*/1));
	EXPECT_EQ(RESULT_OK, eft.setFunctionNumberOfTerms(/*functionNumber*/4, 0));
	EXPECT_EQ(0, eft.getFunctionNumberOfTerms(/*functionNumber*/4));
	EXPECT_EQ(1, eft.getTermLocalNodeIndex(/*functionNumber*/1, /*term*/1));
	EXPECT_EQ(Node::VALUE_LABEL_VALUE, eft.getTermNodeValueLabel(/*functionNumber*/1, /*term*/1));
	EXPECT_EQ(1, eft.getTermNodeVersion(/*functionNumber*/1, /*term*/1));
	EXPECT_EQ(0, eft.getTermLocalNodeIndex(/*functionNumber*/1, /*term*/2));
	EXPECT_EQ(Node::VALUE_LABEL_VALUE, eft.getTermNodeValueLabel(/*functionNumber*/1, /*term*/2));
	EXPECT_EQ(1, eft.getTermNodeVersion(/*functionNumber*/1, /*term*/2));
	EXPECT_EQ(RESULT_OK, eft.setTermNodeParameter(/*functionNumber*/1, /*term*/2, /*localNodeIndex*/2, Node::VALUE_LABEL_VALUE, /*version*/3));

	EXPECT_EQ(2, eft.getTermLocalNodeIndex(/*functionNumber*/1, /*term*/2));
	EXPECT_EQ(Node::VALUE_LABEL_VALUE, eft.getTermNodeValueLabel(/*functionNumber*/1, /*term*/2));
	EXPECT_EQ(3, eft.getTermNodeVersion(/*functionNumber*/1, /*term*/2));
	for (int i = 2; i <= 3; ++i)
	{
		EXPECT_EQ(i, eft.getTermLocalNodeIndex(/*functionNumber*/i, /*term*/1));
		EXPECT_EQ(Node::VALUE_LABEL_VALUE, eft.getTermNodeValueLabel(/*functionNumber*/i, /*term*/1));
		EXPECT_EQ(1, eft.getTermNodeVersion(/*functionNumber*/i, /*term*/1));
	}
	// invalid arguments:
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, eft.setTermNodeParameter(/*functionNumber*/1, /*term*/1, 0, Node::VALUE_LABEL_VALUE, 1));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, eft.setTermNodeParameter(/*functionNumber*/1, /*term*/1, 5, Node::VALUE_LABEL_VALUE, 1));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, eft.setTermNodeParameter(/*functionNumber*/1, /*term*/1, 1, Node::VALUE_LABEL_INVALID, 1));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, eft.setTermNodeParameter(/*functionNumber*/1, /*term*/1, 1, Node::VALUE_LABEL_VALUE, 0));
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, eft.setTermNodeParameter(/*functionNumber*/4, /*term*/1, 1, Node::VALUE_LABEL_VALUE, 1)); // no terms

	// test modifications - scaling
	const int node1term1scaling[2] = { 1, 2 };
	const int node1term2scaling[2] = { 2, 3 };
	const int node2scaling = 2;
	const int node3scaling = 4;
	EXPECT_EQ(RESULT_OK, eft.setTermScaling(/*functionNumber*/1, /*term*/1, 2, node1term1scaling));
	EXPECT_EQ(RESULT_OK, eft.setTermScaling(/*functionNumber*/1, /*term*/2, 2, node1term2scaling));
	EXPECT_EQ(RESULT_OK, eft.setTermScaling(/*functionNumber*/3, /*term*/1, 1, &node3scaling));
	EXPECT_EQ(2, eft.getTermScaling(/*functionNumber*/1, /*term*/1, 0, 0));
	EXPECT_EQ(2, eft.getTermScaling(/*functionNumber*/1, /*term*/2, 0, 0));
	EXPECT_EQ(0, eft.getTermScaling(/*functionNumber*/2, /*term*/1, 0, 0));
	EXPECT_EQ(1, eft.getTermScaling(/*functionNumber*/3, /*term*/1, 0, 0));
	int node1term1scalingOut[2];
	int node1term2scalingOut[2];
	int node2scalingOut;
	int node3scalingOut;
	EXPECT_EQ(2, eft.getTermScaling(/*functionNumber*/1, /*term*/1, 2, node1term1scalingOut));
	EXPECT_EQ(2, eft.getTermScaling(/*functionNumber*/1, /*term*/2, 2, node1term2scalingOut));
	EXPECT_EQ(0, eft.getTermScaling(/*functionNumber*/2, /*term*/1, 1, &node2scalingOut));
	EXPECT_EQ(1, eft.getTermScaling(/*functionNumber*/3, /*term*/1, 1, &node3scalingOut));
	EXPECT_EQ(node1term1scaling[0], node1term1scalingOut[0]);
	EXPECT_EQ(node1term1scaling[1], node1term1scalingOut[1]);
	EXPECT_EQ(node1term2scaling[0], node1term2scalingOut[0]);
	EXPECT_EQ(node1term2scaling[1], node1term2scalingOut[1]);
	EXPECT_EQ(node3scaling, node3scalingOut);
	EXPECT_EQ(RESULT_OK, eft.setTermScaling(/*functionNumber*/2, /*term*/1, 1, &node2scaling));
	EXPECT_EQ(2, eft.getTermScaling(/*functionNumber*/1, /*term*/1, 2, node1term1scalingOut));
	EXPECT_EQ(2, eft.getTermScaling(/*functionNumber*/1, /*term*/2, 2, node1term2scalingOut));
	EXPECT_EQ(1, eft.getTermScaling(/*functionNumber*/2, /*term*/1, 1, &node2scalingOut));
	EXPECT_EQ(1, eft.getTermScaling(/*functionNumber*/3, /*term*/1, 1, &node3scalingOut));
	EXPECT_EQ(node1term1scaling[0], node1term1scalingOut[0]);
	EXPECT_EQ(node1term1scaling[1], node1term1scalingOut[1]);
	EXPECT_EQ(node1term2scaling[0], node1term2scalingOut[0]);
	EXPECT_EQ(node1term2scaling[1], node1term2scalingOut[1]);
	EXPECT_EQ(node2scaling, node2scalingOut);
	EXPECT_EQ(node3scaling, node3scalingOut);
	EXPECT_EQ(RESULT_OK, eft.setTermScaling(/*functionNumber*/2, /*term*/1, 0, 0));
	EXPECT_EQ(2, eft.getTermScaling(/*functionNumber*/1, /*term*/1, 2, node1term1scalingOut));
	EXPECT_EQ(2, eft.getTermScaling(/*functionNumber*/1, /*term*/2, 2, node1term2scalingOut));
	EXPECT_EQ(0, eft.getTermScaling(/*functionNumber*/2, /*term*/1, 1, &node2scalingOut));
	EXPECT_EQ(1, eft.getTermScaling(/*functionNumber*/3, /*term*/1, 1, &node3scalingOut));
	EXPECT_EQ(node1term1scaling[0], node1term1scalingOut[0]);
	EXPECT_EQ(node1term1scaling[1], node1term1scalingOut[1]);
	EXPECT_EQ(node1term2scaling[0], node1term2scalingOut[0]);
	EXPECT_EQ(node1term2scaling[1], node1term2scalingOut[1]);
	EXPECT_EQ(node3scaling, node3scalingOut);
	EXPECT_EQ(RESULT_OK, eft.setTermScaling(/*functionNumber*/2, /*term*/1, 1, &node2scaling));
	EXPECT_EQ(RESULT_OK, eft.setTermScaling(/*functionNumber*/3, /*term*/1, 0, 0));
	EXPECT_EQ(2, eft.getTermScaling(/*functionNumber*/1, /*term*/1, 2, node1term1scalingOut));
	EXPECT_EQ(2, eft.getTermScaling(/*functionNumber*/1, /*term*/2, 2, node1term2scalingOut));
	EXPECT_EQ(1, eft.getTermScaling(/*functionNumber*/2, /*term*/1, 1, &node2scalingOut));
	EXPECT_EQ(0, eft.getTermScaling(/*functionNumber*/3, /*term*/1, 1, &node3scalingOut));
	EXPECT_EQ(node1term1scaling[0], node1term1scalingOut[0]);
	EXPECT_EQ(node1term1scaling[1], node1term1scalingOut[1]);
	EXPECT_EQ(node1term2scaling[0], node1term2scalingOut[0]);
	EXPECT_EQ(node1term2scaling[1], node1term2scalingOut[1]);
	EXPECT_EQ(node2scaling, node2scalingOut);
	EXPECT_EQ(RESULT_OK, eft.setTermScaling(/*functionNumber*/2, /*term*/1, 1, &node2scaling));
	EXPECT_EQ(RESULT_OK, eft.setTermScaling(/*functionNumber*/3, /*term*/1, 1, &node3scaling));
	EXPECT_EQ(2, eft.getTermScaling(/*functionNumber*/1, /*term*/1, 2, node1term1scalingOut));
	EXPECT_EQ(2, eft.getTermScaling(/*functionNumber*/1, /*term*/2, 2, node1term2scalingOut));
	EXPECT_EQ(1, eft.getTermScaling(/*functionNumber*/2, /*term*/1, 1, &node2scalingOut));
	EXPECT_EQ(1, eft.getTermScaling(/*functionNumber*/3, /*term*/1, 1, &node3scalingOut));
	EXPECT_EQ(node1term1scaling[0], node1term1scalingOut[0]);
	EXPECT_EQ(node1term1scaling[1], node1term1scalingOut[1]);
	EXPECT_EQ(node1term2scaling[0], node1term2scalingOut[0]);
	EXPECT_EQ(node1term2scaling[1], node1term2scalingOut[1]);
	EXPECT_EQ(node2scaling, node2scalingOut);
	EXPECT_EQ(node3scaling, node3scalingOut);
	// invalid arguments:
	int dummy;
	EXPECT_EQ(-1, eft.getTermScaling(/*functionNumber*/0, /*term*/1, 1, &dummy));
	EXPECT_EQ(-1, eft.getTermScaling(/*functionNumber*/4, /*term*/1, 1, &dummy));
	EXPECT_EQ(-1, eft.getTermScaling(/*functionNumber*/1, /*term*/0, 1, &dummy));
	EXPECT_EQ(-1, eft.getTermScaling(/*functionNumber*/1, /*term*/3, 1, &dummy));
	EXPECT_EQ(-1, eft.getTermScaling(/*functionNumber*/1, /*term*/3, -1, &dummy));

	// test modifications - terms with scaling
	EXPECT_EQ(RESULT_OK, eft.setFunctionNumberOfTerms(/*functionNumber*/1, 3));
	EXPECT_EQ(3, eft.getFunctionNumberOfTerms(/*functionNumber*/1));
	const int node1term3scaling = 4;
	int node1term3scalingOut;
	EXPECT_EQ(2, eft.getTermScaling(/*functionNumber*/1, /*term*/1, 2, node1term1scalingOut));
	EXPECT_EQ(2, eft.getTermScaling(/*functionNumber*/1, /*term*/2, 2, node1term2scalingOut));
	EXPECT_EQ(0, eft.getTermScaling(/*functionNumber*/1, /*term*/3, 1, &node1term3scalingOut));
	EXPECT_EQ(1, eft.getTermScaling(/*functionNumber*/2, /*term*/1, 1, &node2scalingOut));
	EXPECT_EQ(1, eft.getTermScaling(/*functionNumber*/3, /*term*/1, 1, &node3scalingOut));
	EXPECT_EQ(node1term1scaling[0], node1term1scalingOut[0]);
	EXPECT_EQ(node1term1scaling[1], node1term1scalingOut[1]);
	EXPECT_EQ(node1term2scaling[0], node1term2scalingOut[0]);
	EXPECT_EQ(node1term2scaling[1], node1term2scalingOut[1]);
	EXPECT_EQ(node2scaling, node2scalingOut);
	EXPECT_EQ(node3scaling, node3scalingOut);
	EXPECT_EQ(RESULT_OK, eft.setTermScaling(/*functionNumber*/1, /*term*/3, 1, &node1term3scaling));
	EXPECT_EQ(2, eft.getTermScaling(/*functionNumber*/1, /*term*/1, 2, node1term1scalingOut));
	EXPECT_EQ(2, eft.getTermScaling(/*functionNumber*/1, /*term*/2, 2, node1term2scalingOut));
	EXPECT_EQ(1, eft.getTermScaling(/*functionNumber*/1, /*term*/3, 1, &node1term3scalingOut));
	EXPECT_EQ(1, eft.getTermScaling(/*functionNumber*/2, /*term*/1, 1, &node2scalingOut));
	EXPECT_EQ(1, eft.getTermScaling(/*functionNumber*/3, /*term*/1, 1, &node3scalingOut));
	EXPECT_EQ(node1term1scaling[0], node1term1scalingOut[0]);
	EXPECT_EQ(node1term1scaling[1], node1term1scalingOut[1]);
	EXPECT_EQ(node1term2scaling[0], node1term2scalingOut[0]);
	EXPECT_EQ(node1term2scaling[1], node1term2scalingOut[1]);
	EXPECT_EQ(node1term3scaling, node1term3scalingOut);
	EXPECT_EQ(node2scaling, node2scalingOut);
	EXPECT_EQ(node3scaling, node3scalingOut);
	EXPECT_EQ(RESULT_OK, eft.setFunctionNumberOfTerms(/*functionNumber*/2, 0));
	EXPECT_EQ(0, eft.getFunctionNumberOfTerms(/*functionNumber*/2));
	EXPECT_EQ(2, eft.getTermScaling(/*functionNumber*/1, /*term*/1, 2, node1term1scalingOut));
	EXPECT_EQ(2, eft.getTermScaling(/*functionNumber*/1, /*term*/2, 2, node1term2scalingOut));
	EXPECT_EQ(1, eft.getTermScaling(/*functionNumber*/1, /*term*/3, 1, &node1term3scalingOut));
	EXPECT_EQ(-1, eft.getTermScaling(/*functionNumber*/2, /*term*/1, 1, &node2scalingOut));
	EXPECT_EQ(1, eft.getTermScaling(/*functionNumber*/3, /*term*/1, 1, &node3scalingOut));
	EXPECT_EQ(node1term1scaling[0], node1term1scalingOut[0]);
	EXPECT_EQ(node1term1scaling[1], node1term1scalingOut[1]);
	EXPECT_EQ(node1term2scaling[0], node1term2scalingOut[0]);
	EXPECT_EQ(node1term2scaling[1], node1term2scalingOut[1]);
	EXPECT_EQ(node1term3scaling, node1term3scalingOut);
	EXPECT_EQ(node3scaling, node3scalingOut);
	EXPECT_EQ(RESULT_OK, eft.setTermScaling(/*functionNumber*/1, /*term*/1, 0, 0));
	EXPECT_EQ(RESULT_OK, eft.setFunctionNumberOfTerms(/*functionNumber*/3, 0));
	EXPECT_EQ(0, eft.getFunctionNumberOfTerms(/*functionNumber*/3));
	EXPECT_EQ(0, eft.getTermScaling(/*functionNumber*/1, /*term*/1, 2, node1term1scalingOut));
	EXPECT_EQ(2, eft.getTermScaling(/*functionNumber*/1, /*term*/2, 2, node1term2scalingOut));
	EXPECT_EQ(1, eft.getTermScaling(/*functionNumber*/1, /*term*/3, 1, &node1term3scalingOut));
	EXPECT_EQ(-1, eft.getTermScaling(/*functionNumber*/2, /*term*/1, 1, &node2scalingOut));
	EXPECT_EQ(-1, eft.getTermScaling(/*functionNumber*/3, /*term*/1, 1, &node3scalingOut));
	EXPECT_EQ(node1term2scaling[0], node1term2scalingOut[0]);
	EXPECT_EQ(node1term2scaling[1], node1term2scalingOut[1]);
	EXPECT_EQ(node1term3scaling, node1term3scalingOut);
}

TEST(ZincElementfieldtemplate, node_based_tricubic_Hermite)
{
	ZincTestSetupCpp zinc;

	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_TRUE(mesh3d.isValid());

	// test default node interpolation
	Elementbasis tricubicHermiteBasis = zinc.fm.createElementbasis(3, Elementbasis::FUNCTION_TYPE_CUBIC_HERMITE);
	EXPECT_TRUE(tricubicHermiteBasis.isValid());
	Elementfieldtemplate eft = mesh3d.createElementfieldtemplate(tricubicHermiteBasis);
	EXPECT_TRUE(eft.isValid());
	EXPECT_EQ(Elementfieldtemplate::PARAMETER_MAPPING_MODE_NODE, eft.getParameterMappingMode());

	EXPECT_EQ(64, eft.getNumberOfFunctions());
	EXPECT_EQ(8, eft.getNumberOfLocalNodes());
	EXPECT_EQ(0, eft.getNumberOfLocalScaleFactors());
	int fn = 1;
	for (int no = 1; no <= 8; ++no)
	{
		for (int va = 1; va <= 8; ++va)
		{
			EXPECT_EQ(1, eft.getFunctionNumberOfTerms(fn));
			EXPECT_EQ(no, eft.getTermLocalNodeIndex(/*functionNumber*/fn, /*term*/1));
			EXPECT_EQ(static_cast<Node::ValueLabel>(va), eft.getTermNodeValueLabel(/*functionNumber*/fn, /*term*/1));
			EXPECT_EQ(1, eft.getTermNodeVersion(/*functionNumber*/fn, /*term*/1));
			++fn;
		}
	}
}

// Test EFT validate fails for non-zero identifier for element-based scale factor
// Test EFT validate fails for zero identifier for non-element-based scale factor
TEST(ZincElementfieldtemplate, validate_scale_factor_identifier)
{
	ZincTestSetupCpp zinc;

	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	EXPECT_TRUE(mesh2d.isValid());

	Elementbasis bilinearBasis = zinc.fm.createElementbasis(2, Elementbasis::FUNCTION_TYPE_LINEAR_LAGRANGE);
	EXPECT_TRUE(bilinearBasis.isValid());
	Elementfieldtemplate eft = mesh2d.createElementfieldtemplate(bilinearBasis);
	EXPECT_TRUE(eft.isValid());
	EXPECT_EQ(Elementfieldtemplate::PARAMETER_MAPPING_MODE_NODE, eft.getParameterMappingMode());

	EXPECT_EQ(RESULT_OK, eft.setNumberOfLocalScaleFactors(1));
	EXPECT_EQ(Elementfieldtemplate::SCALE_FACTOR_TYPE_ELEMENT_GENERAL, eft.getScaleFactorType(1));
	EXPECT_EQ(0, eft.getScaleFactorIdentifier(1));
	EXPECT_TRUE(eft.validate());
	EXPECT_EQ(RESULT_OK, eft.setScaleFactorIdentifier(1, 1));
	EXPECT_EQ(1, eft.getScaleFactorIdentifier(1));
	EXPECT_FALSE(eft.validate());
	EXPECT_EQ(RESULT_OK, eft.setScaleFactorType(1, Elementfieldtemplate::SCALE_FACTOR_TYPE_GLOBAL_GENERAL));
	EXPECT_EQ(Elementfieldtemplate::SCALE_FACTOR_TYPE_GLOBAL_GENERAL, eft.getScaleFactorType(1));
	EXPECT_EQ(1, eft.getScaleFactorIdentifier(1));
	EXPECT_TRUE(eft.validate());
	EXPECT_EQ(RESULT_OK, eft.setScaleFactorIdentifier(1, 0));
	EXPECT_EQ(0, eft.getScaleFactorIdentifier(1));
	EXPECT_FALSE(eft.validate());
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

// tests that destroying an element frees its nodes to be destroyed if not in use by other elements
// tests that destroying an element notifies change to mesh and fields
TEST(ZincMesh, destroyElement)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(RESULT_OK, result = zinc.root_region.readFile(TestResources::getLocation(TestResources::FIELDMODULE_TWO_CUBES_RESOURCE)));

	Fieldmodulenotifier notifier = zinc.fm.createFieldmodulenotifier();
	EXPECT_TRUE(notifier.isValid());
	FieldmodulecallbackRecordChange recordChange;
	EXPECT_EQ(RESULT_OK, result = notifier.setCallback(recordChange));
	EXPECT_FALSE(recordChange.lastEvent.isValid());

	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_TRUE(mesh3d.isValid());
	EXPECT_EQ(2, mesh3d.getSize());
	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	EXPECT_TRUE(mesh2d.isValid());
	EXPECT_EQ(11, mesh2d.getSize());
	Mesh mesh1d = zinc.fm.findMeshByDimension(1);
	EXPECT_TRUE(mesh1d.isValid());
	EXPECT_EQ(20, mesh1d.getSize());
	Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodes.isValid());
	EXPECT_EQ(12, nodes.getSize());

	// test can't destroy nodes in use by element
	EXPECT_EQ(RESULT_ERROR_IN_USE, result = nodes.destroyAllNodes());
	EXPECT_EQ(12, nodes.getSize());
	// check above did not notify
	EXPECT_FALSE(recordChange.lastEvent.isValid());

	// now destroy an element
	Element element1 = mesh3d.findElementByIdentifier(1);
	EXPECT_TRUE(element1.isValid());
	Element element2 = mesh3d.findElementByIdentifier(2);
	EXPECT_TRUE(element2.isValid());
	EXPECT_EQ(RESULT_OK, result = mesh3d.destroyElement(element1));
	// element1 should be safely orphaned by the above
	EXPECT_EQ(1, mesh3d.getSize());
	EXPECT_EQ(6, mesh2d.getSize());
	EXPECT_EQ(12, mesh1d.getSize());
	EXPECT_EQ(12, nodes.getSize());

	// test for correct change notification
	EXPECT_TRUE(recordChange.lastEvent.isValid());
	int summaryFieldChange = recordChange.lastEvent.getSummaryFieldChangeFlags();
	EXPECT_EQ(Field::CHANGE_FLAG_PARTIAL_RESULT, summaryFieldChange);
	int coordinatesFieldChange = recordChange.lastEvent.getFieldChangeFlags(coordinates);
	EXPECT_EQ(Field::CHANGE_FLAG_PARTIAL_RESULT, coordinatesFieldChange);
	Meshchanges meshchanges3d = recordChange.lastEvent.getMeshchanges(mesh3d);
	EXPECT_TRUE(meshchanges3d.isValid());
	EXPECT_EQ(1, meshchanges3d.getNumberOfChanges());
	int summaryElementChanges3d = meshchanges3d.getSummaryElementChangeFlags();
	EXPECT_EQ(Element::CHANGE_FLAG_REMOVE, summaryElementChanges3d);
	EXPECT_EQ(Element::CHANGE_FLAG_NONE, meshchanges3d.getElementChangeFlags(element2));
	Meshchanges meshchanges2d = recordChange.lastEvent.getMeshchanges(mesh2d);
	EXPECT_TRUE(meshchanges2d.isValid());
	EXPECT_EQ(5, meshchanges2d.getNumberOfChanges());
	int summaryElementChanges2d = meshchanges2d.getSummaryElementChangeFlags();
	EXPECT_EQ(Element::CHANGE_FLAG_REMOVE, summaryElementChanges2d);
	Meshchanges meshchanges1d = recordChange.lastEvent.getMeshchanges(mesh1d);
	EXPECT_TRUE(meshchanges1d.isValid());
	EXPECT_EQ(8, meshchanges1d.getNumberOfChanges());
	int summaryElementChanges1d = meshchanges1d.getSummaryElementChangeFlags();
	EXPECT_EQ(Element::CHANGE_FLAG_REMOVE, summaryElementChanges1d);
	Nodesetchanges nodesetchanges = recordChange.lastEvent.getNodesetchanges(nodes);
	EXPECT_TRUE(nodesetchanges.isValid());
	EXPECT_EQ(0, nodesetchanges.getNumberOfChanges());
	int summaryNodeChanges = nodesetchanges.getSummaryNodeChangeFlags();
	EXPECT_EQ(Node::CHANGE_FLAG_NONE, summaryNodeChanges);

	// test can destroy nodes no longer in use by element1, but not those in element2
	EXPECT_EQ(RESULT_ERROR_IN_USE, result = nodes.destroyAllNodes());
	EXPECT_EQ(8, nodes.getSize());
	// check the right nodes were destroyed
	for (int n = 0; n < 12; ++n)
	{
		Node node = nodes.findNodeByIdentifier(n + 1);
		if ((n % 3) > 0)
			EXPECT_TRUE(node.isValid());
		else
			EXPECT_FALSE(node.isValid());
	}

	// check above did notify
	EXPECT_TRUE(recordChange.lastEvent.isValid());
	summaryFieldChange = recordChange.lastEvent.getSummaryFieldChangeFlags();
	EXPECT_EQ(Field::CHANGE_FLAG_PARTIAL_RESULT, summaryFieldChange);
	coordinatesFieldChange = recordChange.lastEvent.getFieldChangeFlags(coordinates);
	EXPECT_EQ(Field::CHANGE_FLAG_PARTIAL_RESULT, coordinatesFieldChange);
	nodesetchanges = recordChange.lastEvent.getNodesetchanges(nodes);
	EXPECT_TRUE(nodesetchanges.isValid());
	EXPECT_EQ(4, nodesetchanges.getNumberOfChanges());
	summaryNodeChanges = nodesetchanges.getSummaryNodeChangeFlags();
	EXPECT_EQ(Node::CHANGE_FLAG_REMOVE, summaryNodeChanges);

	// now destroy remaining elements
	EXPECT_EQ(RESULT_OK, result = mesh3d.destroyAllElements());
	// element2 should be safely orphaned by the above
	EXPECT_EQ(0, mesh3d.getSize());
	EXPECT_EQ(0, mesh2d.getSize());
	EXPECT_EQ(0, mesh1d.getSize());
	EXPECT_EQ(8, nodes.getSize());

	// test for correct change notification
	EXPECT_TRUE(recordChange.lastEvent.isValid());
	summaryFieldChange = recordChange.lastEvent.getSummaryFieldChangeFlags();
	EXPECT_EQ(Field::CHANGE_FLAG_PARTIAL_RESULT, summaryFieldChange);
	coordinatesFieldChange = recordChange.lastEvent.getFieldChangeFlags(coordinates);
	EXPECT_EQ(Field::CHANGE_FLAG_PARTIAL_RESULT, coordinatesFieldChange);
	meshchanges3d = recordChange.lastEvent.getMeshchanges(mesh3d);
	EXPECT_TRUE(meshchanges3d.isValid());
	EXPECT_EQ(1, meshchanges3d.getNumberOfChanges());
	summaryElementChanges3d = meshchanges3d.getSummaryElementChangeFlags();
	EXPECT_EQ(Element::CHANGE_FLAG_REMOVE, summaryElementChanges3d);
	meshchanges2d = recordChange.lastEvent.getMeshchanges(mesh2d);
	EXPECT_TRUE(meshchanges2d.isValid());
	EXPECT_EQ(6, meshchanges2d.getNumberOfChanges());
	summaryElementChanges2d = meshchanges2d.getSummaryElementChangeFlags();
	EXPECT_EQ(Element::CHANGE_FLAG_REMOVE, summaryElementChanges2d);
	meshchanges1d = recordChange.lastEvent.getMeshchanges(mesh1d);
	EXPECT_TRUE(meshchanges1d.isValid());
	EXPECT_EQ(12, meshchanges1d.getNumberOfChanges());
	summaryElementChanges1d = meshchanges1d.getSummaryElementChangeFlags();
	EXPECT_EQ(Element::CHANGE_FLAG_REMOVE, summaryElementChanges1d);
	nodesetchanges = recordChange.lastEvent.getNodesetchanges(nodes);
	EXPECT_TRUE(nodesetchanges.isValid());
	EXPECT_EQ(0, nodesetchanges.getNumberOfChanges());
	summaryNodeChanges = nodesetchanges.getSummaryNodeChangeFlags();
	EXPECT_EQ(Node::CHANGE_FLAG_NONE, summaryNodeChanges);

	// test can destroy all nodes since no longer used by elements
	EXPECT_EQ(RESULT_OK, result = nodes.destroyAllNodes());
	EXPECT_EQ(0, nodes.getSize());

	// check above did notify
	EXPECT_TRUE(recordChange.lastEvent.isValid());
	summaryFieldChange = recordChange.lastEvent.getSummaryFieldChangeFlags();
	EXPECT_EQ(Field::CHANGE_FLAG_PARTIAL_RESULT, summaryFieldChange);
	coordinatesFieldChange = recordChange.lastEvent.getFieldChangeFlags(coordinates);
	EXPECT_EQ(Field::CHANGE_FLAG_PARTIAL_RESULT, coordinatesFieldChange);
	nodesetchanges = recordChange.lastEvent.getNodesetchanges(nodes);
	EXPECT_TRUE(nodesetchanges.isValid());
	EXPECT_EQ(8, nodesetchanges.getNumberOfChanges());
	summaryNodeChanges = nodesetchanges.getSummaryNodeChangeFlags();
	EXPECT_EQ(Node::CHANGE_FLAG_REMOVE, summaryNodeChanges);
}

namespace {

Elementfieldtemplate makeScaledTrilinearEft(Fieldmodule& fm)
{
	Mesh mesh3d = fm.findMeshByDimension(3);
	Elementbasis trilinearBasis = fm.createElementbasis(3, Elementbasis::FUNCTION_TYPE_LINEAR_LAGRANGE);
	EXPECT_TRUE(trilinearBasis.isValid());
	Elementfieldtemplate eft = mesh3d.createElementfieldtemplate(trilinearBasis);
	EXPECT_EQ(RESULT_OK, eft.setNumberOfLocalScaleFactors(8));
	EXPECT_TRUE(eft.isValid());
	for (int f = 1; f <= 8; ++f)
	{
		EXPECT_EQ(RESULT_OK, eft.setTermScaling(f, 1, 1, &f));
	}
	return eft;
}

}

// test that when a newly created Elementfieldtemplate is used to get/set element nodes or
// scale factors, it is automatically merged to get matching internal EFT, if it exists
TEST(ZincElementfieldtemplate, mergeExternalElementfieldtemplate)
{
	ZincTestSetupCpp zinc;

	FieldFiniteElement coordinates = zinc.fm.createFieldFiniteElement(/*numberOfComponents*/3);
	EXPECT_TRUE(coordinates.isValid());
	EXPECT_EQ(RESULT_OK, coordinates.setName("coordinates"));
	EXPECT_EQ(RESULT_OK, coordinates.setTypeCoordinate(true));
	EXPECT_EQ(RESULT_OK, coordinates.setCoordinateSystemType(Field::COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN));
	EXPECT_EQ(RESULT_OK, coordinates.setManaged(true));
	EXPECT_EQ(RESULT_OK, coordinates.setComponentName(1, "x"));
	EXPECT_EQ(RESULT_OK, coordinates.setComponentName(2, "y"));
	EXPECT_EQ(RESULT_OK, coordinates.setComponentName(3, "z"));

	Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodes.isValid());
	Nodetemplate nodetemplate = nodes.createNodetemplate();
	EXPECT_TRUE(nodetemplate.isValid());
	EXPECT_EQ(RESULT_OK, nodetemplate.defineField(coordinates));

	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	Elementfieldtemplate eft = makeScaledTrilinearEft(zinc.fm);
	EXPECT_TRUE(eft.isValid());

	Elementtemplate elementtemplate = mesh3d.createElementtemplate();
	EXPECT_TRUE(elementtemplate.isValid());
	EXPECT_EQ(RESULT_OK, elementtemplate.setElementShapeType(Element::SHAPE_TYPE_CUBE));
	EXPECT_EQ(RESULT_OK, elementtemplate.defineField(coordinates, -1, eft));

	Fieldcache cache = zinc.fm.createFieldcache();
	EXPECT_TRUE(cache.isValid());

	int nodeIdentifier = 1;
	for (int i = 0; i < 8; ++i)
	{
		const double x[3] = { static_cast<double>(i%2), static_cast<double>(i%4/2), static_cast<double>(i/4) };
		Node node = nodes.createNode(nodeIdentifier++, nodetemplate);
		EXPECT_EQ(RESULT_OK, cache.setNode(node));
		EXPECT_TRUE(node.isValid());
		EXPECT_EQ(RESULT_OK, coordinates.setNodeParameters(cache, -1, Node::VALUE_LABEL_VALUE, 1, 3, x));
	}

	const int nodeIdentifiers[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
	const double scaleFactors[8] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0 };

	Element element1 = mesh3d.createElement(1, elementtemplate);
	EXPECT_EQ(RESULT_OK, element1.setNodesByIdentifier(eft, 8, nodeIdentifiers));
	EXPECT_EQ(RESULT_OK, element1.setScaleFactors(eft, 8, scaleFactors));

	// test newly created element field template is properly merged
	Node node1 = element1.getNode(eft, 1);
	EXPECT_TRUE(node1.isValid());
	EXPECT_EQ(RESULT_OK, element1.setNode(eft, 1, node1));
	Elementfieldtemplate efta = makeScaledTrilinearEft(zinc.fm);
	Node node1a = element1.getNode(efta, 1);
	EXPECT_EQ(node1, node1a);
	Elementfieldtemplate eftb = makeScaledTrilinearEft(zinc.fm);
	EXPECT_EQ(RESULT_OK, element1.setNode(eftb, 1, node1));
	Elementfieldtemplate eftc = makeScaledTrilinearEft(zinc.fm);
	EXPECT_EQ(RESULT_OK, element1.setNodesByIdentifier(eftc, 8, nodeIdentifiers));

	double scaleFactorOut, scaleFactorsOut[8];
	Elementfieldtemplate eftd = makeScaledTrilinearEft(zinc.fm);
	EXPECT_EQ(RESULT_OK, element1.getScaleFactor(eftd, 1, &scaleFactorOut));
	EXPECT_DOUBLE_EQ(1.0, scaleFactorOut);
	Elementfieldtemplate efte = makeScaledTrilinearEft(zinc.fm);
	EXPECT_EQ(RESULT_OK, element1.setScaleFactor(efte, 1, scaleFactors[0]));
	Elementfieldtemplate eftf = makeScaledTrilinearEft(zinc.fm);
	EXPECT_EQ(RESULT_OK, element1.getScaleFactors(eftf, 8, scaleFactorsOut));
	Elementfieldtemplate eftg = makeScaledTrilinearEft(zinc.fm);
	EXPECT_EQ(RESULT_OK, element1.setScaleFactors(eftg, 8, scaleFactors));
	for (int s = 0; s < 8; ++s)
	{
		EXPECT_DOUBLE_EQ(scaleFactors[s], scaleFactorsOut[s]);
	}
}
