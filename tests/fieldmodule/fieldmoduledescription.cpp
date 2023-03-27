/*
 * Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <cmlibs/zinc/core.h>
#include <cmlibs/zinc/context.h>
#include <cmlibs/zinc/element.h>
#include <cmlibs/zinc/field.h>
#include <cmlibs/zinc/fieldcache.h>
#include <cmlibs/zinc/fieldmodule.h>
#include <cmlibs/zinc/fieldarithmeticoperators.h>
#include <cmlibs/zinc/fieldcomposite.h>
#include <cmlibs/zinc/fieldconstant.h>
#include <cmlibs/zinc/fieldconditional.h>
#include <cmlibs/zinc/fieldcoordinatetransformation.h>
#include <cmlibs/zinc/fieldderivatives.h>
#include <cmlibs/zinc/fieldfiniteelement.h>
#include <cmlibs/zinc/fieldgroup.h>
#include <cmlibs/zinc/fieldlogicaloperators.h>
#include <cmlibs/zinc/fieldmatrixoperators.h>
#include <cmlibs/zinc/fieldmeshoperators.h>
#include <cmlibs/zinc/fieldnodesetoperators.h>
#include <cmlibs/zinc/fieldsubobjectgroup.h>
#include <cmlibs/zinc/fieldtrigonometry.h>
#include <cmlibs/zinc/fieldvectoroperators.h>
#include <cmlibs/zinc/fieldtime.h>
#include <cmlibs/zinc/mesh.h>
#include <cmlibs/zinc/node.h>
#include <cmlibs/zinc/nodeset.h>
#include <cmlibs/zinc/region.h>
#include <cmlibs/zinc/status.h>
#include <cmlibs/zinc/stream.h>
#include <cmlibs/zinc/streamregion.h>
#include <cmlibs/zinc/timekeeper.h>

#include <cmlibs/zinc/field.hpp>
#include <cmlibs/zinc/fieldfiniteelement.hpp>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

#include <string>       // std::string
#include <iostream>     // std::cout, std::ostream, std::hex
#include <sstream>
#include <fstream>

#include "test_resources.h"

void testFields(cmzn_fieldmodule_id fieldmodule)
{
	cmzn_field_id coordinatesField = cmzn_fieldmodule_find_field_by_name(fieldmodule, "coordinates");
	EXPECT_NE(static_cast<cmzn_field *>(0), coordinatesField);
	EXPECT_TRUE(cmzn_field_has_class_name(coordinatesField, "FieldFiniteElement"));

	cmzn_field_id temperatureField = cmzn_fieldmodule_find_field_by_name(fieldmodule, "temperature");
	EXPECT_NE(static_cast<cmzn_field *>(0), temperatureField);
	EXPECT_TRUE(cmzn_field_has_class_name(temperatureField, "FieldFiniteElement"));

	cmzn_field_id componentField = cmzn_fieldmodule_find_field_by_name(
		fieldmodule, "component");
	EXPECT_NE(static_cast<cmzn_field *>(0), componentField);
	EXPECT_TRUE(cmzn_field_has_class_name(componentField, "FieldComponent"));
	EXPECT_TRUE(cmzn_field_is_managed(componentField));
	EXPECT_EQ(1, cmzn_field_get_number_of_source_fields(componentField));
	EXPECT_EQ(2, cmzn_field_get_number_of_components(componentField));
	cmzn_field_id temp = cmzn_field_get_source_field(componentField, 1);
	EXPECT_EQ(temp, coordinatesField);
	cmzn_field_destroy(&temp);

	cmzn_field_component_id fieldComponent = cmzn_field_cast_component(componentField);
	EXPECT_NE(static_cast<cmzn_field_component *>(0), fieldComponent);
	EXPECT_EQ(2, cmzn_field_component_get_source_component_index(fieldComponent, 1));
	EXPECT_EQ(1, cmzn_field_component_get_source_component_index(fieldComponent, 2));
	cmzn_field_component_destroy(&fieldComponent);

	cmzn_field_id firstComponentField = cmzn_fieldmodule_find_field_by_name(
		fieldmodule, "firstComponent");
	cmzn_field_id secondComponentField = cmzn_fieldmodule_find_field_by_name(
		fieldmodule, "secondComponent");
	cmzn_field_id thirdComponentField = cmzn_fieldmodule_find_field_by_name(
		fieldmodule, "thirdComponent");
	cmzn_field_id componentFields[3] = { firstComponentField, secondComponentField, thirdComponentField };
	for (int c = 0; c < 3; ++c)
	{
		EXPECT_NE(nullptr, componentFields[c]);
		EXPECT_TRUE(cmzn_field_has_class_name(componentFields[c], "FieldComponent"));
		EXPECT_TRUE(cmzn_field_is_managed(componentFields[c]));
		EXPECT_EQ(1, cmzn_field_get_number_of_source_fields(componentFields[c]));
		EXPECT_EQ(1, cmzn_field_get_number_of_components(componentFields[c]));
		temp = cmzn_field_get_source_field(componentFields[c], 1);
		EXPECT_EQ(temp, coordinatesField);
		cmzn_field_destroy(&temp);
		cmzn_field_component_id component = cmzn_field_cast_component(componentFields[c]);
		EXPECT_NE(nullptr, component);
		EXPECT_EQ(c + 1, cmzn_field_component_get_component_index(component));
		cmzn_field_component_destroy(&component);
	}

	struct
	{
		const char *name;
		const int componentCount;
	} operators[30] =
	{
		// arithmetic operators
		{ "Abs", 1 },
		{ "Add", 2 },
		{ "Divide", 2 },
		{ "Exp", 1 },
		{ "Log", 1 },
		{ "Multiply", 2 },
		{ "Power", 2 },
		{ "Sqrt", 1 },
		{ "Subtract", 2 },
		// composite
		{ "Identity", 1 },
		// conditional operators
		{ "If", 3 },
		// logical operators
		{ "And", 2 },
		{ "EqualTo", 2 },
		{ "GreaterThan", 2 },
		{ "IsDefined", 1 },
		{ "LessThan", 2 },
		{ "Not", 1 },
		{ "Or", 2 },
		{ "Xor", 2 },
		// trigonometric operators
		{ "Sin", 1 },
		{ "Cos", 1 },
		{ "Tan", 1 },
		{ "Asin", 1 },
		{ "Acos", 1 },
		{ "Atan", 1 },
		{ "Atan2", 2 },
		// vector operators
		{ "DotProduct", 2 },
		{ "Magnitude", 1 },
		{ "Normalise", 1 },
		{ "SumComponents", 1 }
	};
    for (int f = 0; f < 30; ++f)
    {
        const char *name = operators[f].name;
        cmzn_field_id field = cmzn_fieldmodule_find_field_by_name(fieldmodule, name);
        EXPECT_NE(static_cast<cmzn_field *>(0), field);
        std::string expectedClassName = "Field";
        expectedClassName += name;
        EXPECT_TRUE(cmzn_field_has_class_name(field, expectedClassName.c_str()));
        EXPECT_TRUE(cmzn_field_is_managed(field));
        EXPECT_EQ(operators[f].componentCount, cmzn_field_get_number_of_source_fields(field));
        EXPECT_EQ(1, cmzn_field_get_number_of_components(field));
        temp = cmzn_field_get_source_field(field, 1);
        EXPECT_EQ(firstComponentField, temp);
        cmzn_field_destroy(&temp);
		if (operators[f].componentCount == 3)  // If
		{
			temp = cmzn_field_get_source_field(field, 2);
			EXPECT_EQ(temp, secondComponentField);
			cmzn_field_destroy(&temp);
			temp = cmzn_field_get_source_field(field, 3);
			EXPECT_EQ(temp, thirdComponentField);
			cmzn_field_destroy(&temp);
		}
        else if (operators[f].componentCount > 1)
        {
            temp = cmzn_field_get_source_field(field, 2);
            EXPECT_EQ(temp, temperatureField);
            cmzn_field_destroy(&temp);
        }
        cmzn_field_destroy(&field);
    }

	cmzn_field_id concatenateField = cmzn_fieldmodule_find_field_by_name(
		fieldmodule, "concatenate");
	EXPECT_NE(static_cast<cmzn_field *>(0), concatenateField);
	EXPECT_TRUE(cmzn_field_has_class_name(concatenateField, "FieldConcatenate"));
	EXPECT_TRUE(cmzn_field_is_managed(concatenateField));
	EXPECT_EQ(2, cmzn_field_get_number_of_source_fields(concatenateField));
	EXPECT_EQ(4, cmzn_field_get_number_of_components(concatenateField));
	temp = cmzn_field_get_source_field(concatenateField, 1);
	EXPECT_EQ(temp, coordinatesField);
	cmzn_field_destroy(&temp);
	temp = cmzn_field_get_source_field(concatenateField, 2);
	EXPECT_EQ(temp, temperatureField);
	cmzn_field_destroy(&temp);

	cmzn_fieldcache_id fieldCache = cmzn_fieldmodule_create_fieldcache(fieldmodule);

	cmzn_field_id constantField = cmzn_fieldmodule_find_field_by_name(
		fieldmodule, "constant");
	EXPECT_NE(static_cast<cmzn_field *>(0), constantField);
	EXPECT_TRUE(cmzn_field_has_class_name(constantField, "FieldConstant"));
	EXPECT_TRUE(cmzn_field_is_managed(constantField));
	EXPECT_EQ(0, cmzn_field_get_number_of_source_fields(constantField));
	EXPECT_EQ(3, cmzn_field_get_number_of_components(constantField));
	double values3[3] = { 0.0, 0.0, 0.0 };
	EXPECT_EQ(CMZN_OK, cmzn_field_evaluate_real(constantField, fieldCache, 3, &values3[0]));
	EXPECT_EQ(1.0, values3[0]);
	EXPECT_EQ(4.0, values3[1]);
	EXPECT_EQ(2.0, values3[2]);

	cmzn_field_id stringConstantField = cmzn_fieldmodule_find_field_by_name(
		fieldmodule, "stringConstant");
	EXPECT_NE(static_cast<cmzn_field *>(0), stringConstantField);
	EXPECT_TRUE(cmzn_field_has_class_name(stringConstantField, "FieldStringConstant"));
	EXPECT_TRUE(cmzn_field_is_managed(stringConstantField));
	EXPECT_EQ(0, cmzn_field_get_number_of_source_fields(stringConstantField));
	EXPECT_EQ(1, cmzn_field_get_number_of_components(stringConstantField));
	char *returned_string = cmzn_field_evaluate_string(stringConstantField, fieldCache);
	EXPECT_STREQ(returned_string, "string_constant");
	cmzn_deallocate(returned_string);

	cmzn_field_id edgeDiscontinuityField = cmzn_fieldmodule_find_field_by_name(
		fieldmodule, "edgeDiscontinuity");
	EXPECT_NE(static_cast<cmzn_field *>(0), edgeDiscontinuityField);
	EXPECT_TRUE(cmzn_field_has_class_name(edgeDiscontinuityField, "FieldEdgeDiscontinuity"));
	EXPECT_TRUE(cmzn_field_is_managed(edgeDiscontinuityField));
	EXPECT_EQ(1, cmzn_field_get_number_of_source_fields(edgeDiscontinuityField));

	cmzn_field_id derivativeField = cmzn_fieldmodule_find_field_by_name(fieldmodule,
		"derivative");
	EXPECT_NE(static_cast<cmzn_field *>(0), derivativeField);
	EXPECT_TRUE(cmzn_field_has_class_name(derivativeField, "FieldDerivative"));
	EXPECT_TRUE(cmzn_field_is_managed(derivativeField));
	EXPECT_EQ(1, cmzn_field_get_number_of_source_fields(derivativeField));
	// check the parameters with type-specific API
	cmzn_field_derivative_id derivative = cmzn_field_cast_derivative(derivativeField);
	EXPECT_NE(static_cast<cmzn_field_derivative *>(0), derivative);
	EXPECT_EQ(2, cmzn_field_derivative_get_xi_index(derivative));
	cmzn_field_derivative_destroy(&derivative);

	cmzn_field_id coordinateTransformationField =
		cmzn_fieldmodule_find_field_by_name(fieldmodule, "coordinateTransformation");
	EXPECT_NE(static_cast<cmzn_field *>(0), coordinateTransformationField);
	EXPECT_TRUE(cmzn_field_has_class_name(coordinateTransformationField, "FieldCoordinateTransformation"));
	EXPECT_TRUE(cmzn_field_is_managed(coordinateTransformationField));
	EXPECT_EQ(1, cmzn_field_get_number_of_source_fields(coordinateTransformationField));
	EXPECT_EQ(CMZN_FIELD_COORDINATE_SYSTEM_TYPE_PROLATE_SPHEROIDAL,
		cmzn_field_get_coordinate_system_type(coordinateTransformationField));
	EXPECT_EQ(0.7, cmzn_field_get_coordinate_system_focus(coordinateTransformationField));

	cmzn_field_id determinantField =
		cmzn_fieldmodule_find_field_by_name(fieldmodule, "determinant");
	EXPECT_NE(static_cast<cmzn_field *>(0), determinantField);
	EXPECT_TRUE(cmzn_field_has_class_name(determinantField, "FieldDeterminant"));
	EXPECT_EQ(CMZN_OK, cmzn_field_evaluate_real(determinantField, fieldCache, 1, &values3[0]));
	EXPECT_EQ(-3.0, values3[0]);

	cmzn_field_id matrixMultiplyField =
		cmzn_fieldmodule_find_field_by_name(fieldmodule, "matrixMultiply");
	EXPECT_NE(static_cast<cmzn_field *>(0), matrixMultiplyField);
	EXPECT_TRUE(cmzn_field_has_class_name(matrixMultiplyField, "FieldMatrixMultiply"));
	EXPECT_EQ(9, cmzn_field_get_number_of_components(matrixMultiplyField));

	cmzn_field_id eigenvaluesField =
		cmzn_fieldmodule_find_field_by_name(fieldmodule, "eigenvalues");
	EXPECT_NE(static_cast<cmzn_field *>(0), eigenvaluesField);
	EXPECT_TRUE(cmzn_field_has_class_name(eigenvaluesField, "FieldEigenvalues"));
	EXPECT_EQ(3, cmzn_field_get_number_of_components(eigenvaluesField));

	cmzn_field_id eigenvectorsField =
		cmzn_fieldmodule_find_field_by_name(fieldmodule, "eigenvectors");
	EXPECT_NE(static_cast<cmzn_field *>(0), eigenvectorsField);
	EXPECT_TRUE(cmzn_field_has_class_name(eigenvectorsField, "FieldEigenvectors"));
	EXPECT_EQ(9, cmzn_field_get_number_of_components(eigenvectorsField));

	cmzn_field_id crossProductField = cmzn_fieldmodule_find_field_by_name(
		fieldmodule, "crossProduct");
	EXPECT_TRUE(cmzn_field_has_class_name(crossProductField, "FieldCrossProduct"));
	EXPECT_NE(static_cast<cmzn_field *>(0), crossProductField);

	cmzn_field_id timeValueField = cmzn_fieldmodule_find_field_by_name(
		fieldmodule, "timeValue");
	EXPECT_TRUE(cmzn_field_has_class_name(timeValueField, "FieldTimeValue"));
	EXPECT_NE(static_cast<cmzn_field *>(0), timeValueField);

	cmzn_field_id timeLookupField = cmzn_fieldmodule_find_field_by_name(
		fieldmodule, "timeLookup");
	EXPECT_TRUE(cmzn_field_has_class_name(timeLookupField, "FieldTimeLookup"));
	EXPECT_NE(static_cast<cmzn_field *>(0), timeLookupField);

	cmzn_field_id transposeField = cmzn_fieldmodule_find_field_by_name(
		fieldmodule, "transpose");
	EXPECT_TRUE(cmzn_field_has_class_name(transposeField, "FieldTranspose"));
	EXPECT_NE(static_cast<cmzn_field *>(0), transposeField);

	cmzn_field_id nodeValueField = cmzn_fieldmodule_find_field_by_name(
		fieldmodule, "nodeValue");
	EXPECT_NE(static_cast<cmzn_field *>(0), nodeValueField);
	EXPECT_TRUE(cmzn_field_has_class_name(nodeValueField, "FieldNodeValue"));
	// check the parameters with type-specific API
	cmzn_field_node_value_id nodeValue = cmzn_field_cast_node_value(nodeValueField);
	EXPECT_NE(static_cast<cmzn_field_node_value *>(0), nodeValue);
	EXPECT_EQ(CMZN_NODE_VALUE_LABEL_D_DS1, cmzn_field_node_value_get_node_value_label(nodeValue));
	EXPECT_EQ(2, cmzn_field_node_value_get_version_number(nodeValue));
	cmzn_field_node_value_destroy(&nodeValue);

	cmzn_field_id isOnFaceField = cmzn_fieldmodule_find_field_by_name(
		fieldmodule, "isOnFace");
	EXPECT_NE(static_cast<cmzn_field *>(0), isOnFaceField);
	EXPECT_TRUE(cmzn_field_has_class_name(isOnFaceField, "FieldIsOnFace"));
	// check the element face type with type-specific API
	cmzn_field_is_on_face_id isOnFace = cmzn_field_cast_is_on_face(isOnFaceField);
	EXPECT_NE(static_cast<cmzn_field_is_on_face *>(0), isOnFace);
	EXPECT_EQ(CMZN_ELEMENT_FACE_TYPE_XI1_0, cmzn_field_is_on_face_get_element_face_type(isOnFace));
	cmzn_field_is_on_face_destroy(&isOnFace);

	cmzn_field_id storedMeshLocationField = cmzn_fieldmodule_find_field_by_name(
		fieldmodule, "storedMeshLocation");
	EXPECT_NE(static_cast<cmzn_field *>(0), storedMeshLocationField);
	EXPECT_TRUE(cmzn_field_has_class_name(storedMeshLocationField, "FieldStoredMeshLocation"));

	cmzn_mesh_id mesh3d = cmzn_fieldmodule_find_mesh_by_dimension(fieldmodule, 3);
	cmzn_mesh_id mesh2d = cmzn_fieldmodule_find_mesh_by_dimension(fieldmodule, 2);
	cmzn_field_id triangleField = cmzn_fieldmodule_find_field_by_name(fieldmodule, "triangle");
	cmzn_field_group_id triangleGroup = cmzn_field_cast_group(triangleField);
	cmzn_field_element_group_id triangleElementGroup2d = cmzn_field_group_get_field_element_group(triangleGroup, mesh2d);
	EXPECT_NE(static_cast<cmzn_field_element_group *>(0), triangleElementGroup2d);
	cmzn_mesh_group_id triangleMeshGroup2d = cmzn_field_element_group_get_mesh_group(triangleElementGroup2d);
	EXPECT_EQ(1, cmzn_mesh_get_size(cmzn_mesh_group_base_cast(triangleMeshGroup2d)));
	cmzn_nodeset_id nodes = cmzn_fieldmodule_find_nodeset_by_field_domain_type(fieldmodule, CMZN_FIELD_DOMAIN_TYPE_NODES);
	cmzn_field_node_group_id triangleNodeGroup = cmzn_field_group_get_field_node_group(triangleGroup, nodes);
	EXPECT_NE(static_cast<cmzn_field_node_group *>(0), triangleNodeGroup);
	cmzn_nodeset_group_id triangleNodesetGroup = cmzn_field_node_group_get_nodeset_group(triangleNodeGroup);
	EXPECT_EQ(3, cmzn_nodeset_get_size(cmzn_nodeset_group_base_cast(triangleNodesetGroup)));

	cmzn_mesh_id mesh;
	cmzn_nodeset_id nodeset;
	cmzn_field_id field;

	cmzn_field_id findMeshLocationField = cmzn_fieldmodule_find_field_by_name(
		fieldmodule, "findMeshLocation");
	EXPECT_NE(static_cast<cmzn_field *>(0), findMeshLocationField);
	EXPECT_TRUE(cmzn_field_has_class_name(findMeshLocationField, "FieldFindMeshLocation"));
	cmzn_field_find_mesh_location_id findMeshLocation = cmzn_field_cast_find_mesh_location(findMeshLocationField);
	EXPECT_NE(static_cast<cmzn_field_find_mesh_location *>(0), findMeshLocation);
	mesh = cmzn_field_find_mesh_location_get_mesh(findMeshLocation);
	EXPECT_TRUE(cmzn_mesh_match(mesh3d, mesh));
	cmzn_mesh_destroy(&mesh);
	EXPECT_EQ(CMZN_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_NEAREST, cmzn_field_find_mesh_location_get_search_mode(findMeshLocation));
	mesh = cmzn_field_find_mesh_location_get_search_mesh(findMeshLocation);
	EXPECT_TRUE(cmzn_mesh_match(cmzn_mesh_group_base_cast(triangleMeshGroup2d), mesh));
	cmzn_mesh_destroy(&mesh);
	cmzn_field_find_mesh_location_destroy(&findMeshLocation);
	cmzn_field_destroy(&findMeshLocationField);

	int numbersOfPoints[2];

	cmzn_field_id meshIntegralField = cmzn_fieldmodule_find_field_by_name(
		fieldmodule, "meshIntegral");
	EXPECT_NE(static_cast<cmzn_field *>(0), meshIntegralField);
	EXPECT_TRUE(cmzn_field_has_class_name(meshIntegralField, "FieldMeshIntegral"));
	cmzn_field_mesh_integral_id meshIntegral = cmzn_field_cast_mesh_integral(meshIntegralField);
	EXPECT_NE(static_cast<cmzn_field_mesh_integral *>(0), meshIntegral);
	mesh = cmzn_field_mesh_integral_get_mesh(meshIntegral);
	EXPECT_TRUE(cmzn_mesh_match(cmzn_mesh_group_base_cast(triangleMeshGroup2d), mesh));
	cmzn_mesh_destroy(&mesh);
	EXPECT_EQ(CMZN_ELEMENT_QUADRATURE_RULE_GAUSSIAN, cmzn_field_mesh_integral_get_element_quadrature_rule(meshIntegral));
	EXPECT_EQ(2, cmzn_field_mesh_integral_get_numbers_of_points(meshIntegral, 2, numbersOfPoints));
	EXPECT_EQ(2, numbersOfPoints[0]);
	EXPECT_EQ(4, numbersOfPoints[1]);
	cmzn_field_mesh_integral_destroy(&meshIntegral);
	cmzn_field_destroy(&meshIntegralField);

	cmzn_field_id meshIntegralSquaresField = cmzn_fieldmodule_find_field_by_name(
		fieldmodule, "meshIntegralSquares");
	EXPECT_NE(static_cast<cmzn_field *>(0), meshIntegralSquaresField);
	EXPECT_TRUE(cmzn_field_has_class_name(meshIntegralSquaresField, "FieldMeshIntegralSquares"));
	cmzn_field_mesh_integral_id meshIntegralSquares = cmzn_field_cast_mesh_integral(meshIntegralSquaresField);
	EXPECT_NE(static_cast<cmzn_field_mesh_integral *>(0), meshIntegralSquares);
	mesh = cmzn_field_mesh_integral_get_mesh(meshIntegralSquares);
	EXPECT_TRUE(cmzn_mesh_match(mesh2d, mesh));
	cmzn_mesh_destroy(&mesh);
	EXPECT_EQ(CMZN_ELEMENT_QUADRATURE_RULE_MIDPOINT, cmzn_field_mesh_integral_get_element_quadrature_rule(meshIntegralSquares));
	EXPECT_EQ(2, cmzn_field_mesh_integral_get_numbers_of_points(meshIntegralSquares, 2, numbersOfPoints));
	EXPECT_EQ(3, numbersOfPoints[0]);
	EXPECT_EQ(7, numbersOfPoints[1]);
	cmzn_field_mesh_integral_destroy(&meshIntegralSquares);
	cmzn_field_destroy(&meshIntegralSquaresField);

	const char *nodesetOperatorNames[6] = { "Maximum", "Mean", "MeanSquares", "Minimum", "Sum", "SumSquares" };
	for (int i = 0; i < 6; ++i)
	{
		std::string fieldName("nodeset");
		fieldName += nodesetOperatorNames[i];
		std::string className("FieldNodeset");
		className += nodesetOperatorNames[i];
		cmzn_field_id nodesetOperatorField = cmzn_fieldmodule_find_field_by_name(fieldmodule, fieldName.c_str());
		EXPECT_NE(static_cast<cmzn_field *>(0), nodesetOperatorField);
		EXPECT_TRUE(cmzn_field_has_class_name(nodesetOperatorField, className.c_str()));
		cmzn_field_nodeset_operator_id nodesetOperator = cmzn_field_cast_nodeset_operator(nodesetOperatorField);
		EXPECT_NE(static_cast<cmzn_field_nodeset_operator *>(0), nodesetOperator);
		field = cmzn_field_nodeset_operator_get_element_map_field(nodesetOperator);
		EXPECT_EQ((i == 4) ? storedMeshLocationField : static_cast<cmzn_field *>(0), field);
		cmzn_field_destroy(&field);
		nodeset = cmzn_field_nodeset_operator_get_nodeset(nodesetOperator);
		EXPECT_TRUE(cmzn_nodeset_match((i == 4) ? cmzn_nodeset_group_base_cast(triangleNodesetGroup) : nodes, nodeset));
		cmzn_nodeset_destroy(&nodeset);
		cmzn_field_nodeset_operator_destroy(&nodesetOperator);
		cmzn_field_destroy(&nodesetOperatorField);
	}

	cmzn_nodeset_group_destroy(&triangleNodesetGroup);
	cmzn_field_node_group_destroy(&triangleNodeGroup);
	cmzn_nodeset_destroy(&nodes);
	cmzn_mesh_group_destroy(&triangleMeshGroup2d);
	cmzn_field_element_group_destroy(&triangleElementGroup2d);
	cmzn_field_group_destroy(&triangleGroup);
	cmzn_field_destroy(&triangleField);
	cmzn_mesh_destroy(&mesh2d);
	cmzn_mesh_destroy(&mesh3d);

	cmzn_fieldcache_destroy(&fieldCache);
	cmzn_field_destroy(&storedMeshLocationField);
	cmzn_field_destroy(&isOnFaceField);
	cmzn_field_destroy(&nodeValueField);
	cmzn_field_destroy(&transposeField);
	cmzn_field_destroy(&timeLookupField);
	cmzn_field_destroy(&timeValueField);
	cmzn_field_destroy(&crossProductField);
	cmzn_field_destroy(&eigenvectorsField);
	cmzn_field_destroy(&eigenvaluesField);
    cmzn_field_destroy(&matrixMultiplyField);
	cmzn_field_destroy(&determinantField);
	cmzn_field_destroy(&coordinateTransformationField);
	cmzn_field_destroy(&derivativeField);
	cmzn_field_destroy(&edgeDiscontinuityField);
	cmzn_field_destroy(&stringConstantField);
	cmzn_field_destroy(&constantField);
	cmzn_field_destroy(&concatenateField);
	cmzn_field_destroy(&thirdComponentField);
	cmzn_field_destroy(&secondComponentField);
	cmzn_field_destroy(&firstComponentField);
	cmzn_field_destroy(&componentField);
	cmzn_field_destroy(&temperatureField);
	cmzn_field_destroy(&coordinatesField);
}

void TestDescriptionOutput(cmzn_region_id region, const char *description)
{
	cmzn_streaminformation_id si = cmzn_region_create_streaminformation_region(
		region);
	EXPECT_NE(static_cast<cmzn_streaminformation *>(0), si);

	cmzn_streamresource_id sr = cmzn_streaminformation_create_streamresource_file(
        si, resourcePath("fieldmodule/region_input.exf").c_str());
	EXPECT_NE(static_cast<cmzn_streamresource *>(0), sr);

	cmzn_streaminformation_region_id si_region = cmzn_streaminformation_cast_region(
		si);
	EXPECT_NE(static_cast<cmzn_streaminformation_region *>(0), si_region);

	int result = cmzn_region_read(region, si_region);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_streamresource_destroy(&sr);
	cmzn_streaminformation_region_destroy(&si_region);
	cmzn_streaminformation_destroy(&si);

	cmzn_region_id tetrahedron_region = cmzn_region_find_child_by_name(
		region, "tetrahedron");
	EXPECT_NE(static_cast<cmzn_region *>(0), tetrahedron_region);

	cmzn_fieldmodule_id fieldmodule = cmzn_region_get_fieldmodule(tetrahedron_region);
	EXPECT_NE(static_cast<cmzn_fieldmodule *>(0), fieldmodule);

	EXPECT_EQ(CMZN_OK, cmzn_fieldmodule_read_description(fieldmodule, description));

    testFields(fieldmodule);

    cmzn_region_destroy(&tetrahedron_region);
    cmzn_fieldmodule_destroy(&fieldmodule);
}

TEST(fieldmodule_description, write)
{
	cmzn_context_id context = cmzn_context_create("test");
	cmzn_region_id root_region = cmzn_context_get_default_region(context);

	cmzn_streaminformation_id si = cmzn_region_create_streaminformation_region(
		root_region);
	EXPECT_NE(static_cast<cmzn_streaminformation *>(0), si);

	cmzn_streamresource_id sr = cmzn_streaminformation_create_streamresource_file(
        si, resourcePath("fieldmodule/region_input.exf").c_str());
	EXPECT_NE(static_cast<cmzn_streamresource *>(0), sr);

	cmzn_streaminformation_region_id si_region = cmzn_streaminformation_cast_region(
		si);
	EXPECT_NE(static_cast<cmzn_streaminformation_region *>(0), si_region);

	int result = cmzn_region_read(root_region, si_region);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_streamresource_destroy(&sr);
	cmzn_streaminformation_region_destroy(&si_region);
	cmzn_streaminformation_destroy(&si);

	cmzn_region_id tetrahedron_region = cmzn_region_find_child_by_name(
		root_region, "tetrahedron");
	EXPECT_NE(static_cast<cmzn_region *>(0), tetrahedron_region);

	cmzn_fieldmodule_id fieldmodule = cmzn_region_get_fieldmodule(tetrahedron_region);
	EXPECT_NE(static_cast<cmzn_fieldmodule *>(0), fieldmodule);

	cmzn_field_id coordinatesField = cmzn_fieldmodule_find_field_by_name(fieldmodule, "coordinates");
	EXPECT_NE(static_cast<cmzn_field *>(0), coordinatesField);

	cmzn_field_id temperatureField = cmzn_fieldmodule_find_field_by_name(fieldmodule, "temperature");
	EXPECT_NE(static_cast<cmzn_field *>(0), temperatureField);

	int indexes[2] = {2,1};

	cmzn_field_id componentField = cmzn_fieldmodule_create_field_component_multiple(
		fieldmodule, coordinatesField, 2, indexes);
	cmzn_field_set_managed(componentField, true);
	cmzn_field_set_name(componentField, "component");
	EXPECT_NE(static_cast<cmzn_field *>(0), componentField);

	cmzn_field_id firstComponentField = cmzn_fieldmodule_create_field_component(
		fieldmodule, coordinatesField, 1);
	EXPECT_NE(static_cast<cmzn_field *>(0), firstComponentField);
	cmzn_field_set_managed(firstComponentField, false);
	cmzn_field_set_name(firstComponentField, "firstComponent");

	cmzn_field_id secondComponentField = cmzn_fieldmodule_create_field_component(
		fieldmodule, coordinatesField, 2);
	EXPECT_NE(static_cast<cmzn_field*>(0), secondComponentField);
	cmzn_field_set_managed(secondComponentField, false);
	cmzn_field_set_name(secondComponentField, "secondComponent");

	cmzn_field_id thirdComponentField = cmzn_fieldmodule_create_field_component(
		fieldmodule, coordinatesField, 3);
	EXPECT_NE(static_cast<cmzn_field*>(0), thirdComponentField);
	cmzn_field_set_managed(thirdComponentField, false);
	cmzn_field_set_name(thirdComponentField, "thirdComponent");

	struct
	{
		const char *name;
		cmzn_field_id field;
	} operators[30] =
	{
		{ "Abs", cmzn_fieldmodule_create_field_abs(fieldmodule, firstComponentField) },
		{ "Add", cmzn_fieldmodule_create_field_add(fieldmodule, firstComponentField, temperatureField) },
		{ "Divide", cmzn_fieldmodule_create_field_divide(fieldmodule, firstComponentField, temperatureField) },
		{ "Exp", cmzn_fieldmodule_create_field_exp(fieldmodule, firstComponentField) },
		{ "Log", cmzn_fieldmodule_create_field_log(fieldmodule, firstComponentField) },
		{ "Multiply", cmzn_fieldmodule_create_field_multiply(fieldmodule, firstComponentField, temperatureField) },
		{ "Power", cmzn_fieldmodule_create_field_power(fieldmodule, firstComponentField, temperatureField) },
		{ "Sqrt", cmzn_fieldmodule_create_field_sqrt(fieldmodule, firstComponentField) },
		{ "Subtract", cmzn_fieldmodule_create_field_subtract(fieldmodule, firstComponentField, temperatureField) },
		// composite operators
		{ "Identity", cmzn_fieldmodule_create_field_identity(fieldmodule, firstComponentField) },
		// conditional operators
		{ "If", cmzn_fieldmodule_create_field_if(fieldmodule, firstComponentField, secondComponentField, thirdComponentField) },
		// logical operators
		{ "And", cmzn_fieldmodule_create_field_and(fieldmodule, firstComponentField, temperatureField) },
		{ "EqualTo", cmzn_fieldmodule_create_field_equal_to(fieldmodule, firstComponentField, temperatureField) },
		{ "GreaterThan", cmzn_fieldmodule_create_field_greater_than(fieldmodule, firstComponentField, temperatureField) },
		{ "IsDefined", cmzn_fieldmodule_create_field_is_defined(fieldmodule, firstComponentField) },
		{ "LessThan", cmzn_fieldmodule_create_field_less_than(fieldmodule, firstComponentField, temperatureField) },
		{ "Not", cmzn_fieldmodule_create_field_not(fieldmodule, firstComponentField) },
		{ "Or", cmzn_fieldmodule_create_field_or(fieldmodule, firstComponentField, temperatureField) },
		{ "Xor", cmzn_fieldmodule_create_field_xor(fieldmodule, firstComponentField, temperatureField) },
		// trigonometric operators
		{ "Sin", cmzn_fieldmodule_create_field_sin(fieldmodule, firstComponentField) },
		{ "Cos", cmzn_fieldmodule_create_field_cos(fieldmodule, firstComponentField) },
		{ "Tan", cmzn_fieldmodule_create_field_tan(fieldmodule, firstComponentField) },
		{ "Asin", cmzn_fieldmodule_create_field_asin(fieldmodule, firstComponentField) },
		{ "Acos", cmzn_fieldmodule_create_field_acos(fieldmodule, firstComponentField) },
		{ "Atan", cmzn_fieldmodule_create_field_atan(fieldmodule, firstComponentField) },
		{ "Atan2", cmzn_fieldmodule_create_field_atan2(fieldmodule, firstComponentField, temperatureField) },
		// vector operators
		{ "DotProduct", cmzn_fieldmodule_create_field_dot_product(fieldmodule, firstComponentField, temperatureField) },
		{ "Magnitude", cmzn_fieldmodule_create_field_magnitude(fieldmodule, firstComponentField) },
		{ "Normalise", cmzn_fieldmodule_create_field_normalise(fieldmodule, firstComponentField) },
		{ "SumComponents", cmzn_fieldmodule_create_field_sum_components(fieldmodule, firstComponentField) }
	};
    for (int f = 0; f < 30; ++f)
    {
        const char *name = operators[f].name;
        EXPECT_NE(static_cast<cmzn_field *>(0), operators[f].field);
        std::string expectedClassName = "Field";
        expectedClassName += name;
        EXPECT_TRUE(cmzn_field_has_class_name(operators[f].field, expectedClassName.c_str()));
        EXPECT_NE(static_cast<cmzn_field *>(0), operators[f].field);
        cmzn_field_set_managed(operators[f].field, true);
        cmzn_field_set_name(operators[f].field, operators[f].name);
        cmzn_field_destroy(&operators[f].field);
    }

	cmzn_field_id sourceFields[2] = {coordinatesField, temperatureField};

	cmzn_field_id concatenateField = cmzn_fieldmodule_create_field_concatenate(
		fieldmodule, 2, sourceFields);
	cmzn_field_set_managed(concatenateField, true);
	cmzn_field_set_name(concatenateField, "concatenate");
	EXPECT_NE(static_cast<cmzn_field *>(0), concatenateField);

	double values3[3] = {1.0, 4.0, 2.0};
	cmzn_field_id constantField = cmzn_fieldmodule_create_field_constant(
		fieldmodule, 3, values3);
	cmzn_field_set_managed(constantField, true);
	cmzn_field_set_name(constantField, "constant");
	EXPECT_NE(static_cast<cmzn_field *>(0), constantField);

	cmzn_field_id stringConstantField = cmzn_fieldmodule_create_field_string_constant(
		fieldmodule, "string_constant");
	cmzn_field_set_managed(stringConstantField, true);
	cmzn_field_set_name(stringConstantField, "stringConstant");
	EXPECT_NE(static_cast<cmzn_field *>(0), stringConstantField);

	cmzn_field_id edgeDiscontinuityField = cmzn_fieldmodule_create_field_edge_discontinuity(
		fieldmodule, coordinatesField);
	EXPECT_NE(static_cast<cmzn_field *>(0), edgeDiscontinuityField);
	cmzn_field_set_managed(edgeDiscontinuityField, true);
	cmzn_field_set_name(edgeDiscontinuityField, "edgeDiscontinuity");

	cmzn_field_id derivativeField = cmzn_fieldmodule_create_field_derivative(fieldmodule,
		coordinatesField, 2);
	EXPECT_NE(static_cast<cmzn_field *>(0), derivativeField);
	cmzn_field_set_managed(derivativeField, true);
	cmzn_field_set_name(derivativeField, "derivative");

	cmzn_field_id coordinateTransformationField =
		cmzn_fieldmodule_create_field_coordinate_transformation(
			fieldmodule, coordinatesField);
	EXPECT_NE(static_cast<cmzn_field *>(0), coordinateTransformationField);
	cmzn_field_set_managed(coordinateTransformationField, true);
	cmzn_field_set_name(coordinateTransformationField, "coordinateTransformation");
	cmzn_field_set_coordinate_system_type(coordinateTransformationField,
		CMZN_FIELD_COORDINATE_SYSTEM_TYPE_PROLATE_SPHEROIDAL);
	cmzn_field_set_coordinate_system_focus(coordinateTransformationField, 0.7);

	double values9[9] = {2.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
	cmzn_field_id matrixField = cmzn_fieldmodule_create_field_constant(
		fieldmodule, 9, values9);
	EXPECT_NE(static_cast<cmzn_field *>(0), matrixField);
	cmzn_field_set_name(matrixField, "matrix");

	cmzn_field_id determinantField =
		cmzn_fieldmodule_create_field_determinant(fieldmodule, matrixField);
	EXPECT_NE(static_cast<cmzn_field *>(0), determinantField);
	cmzn_field_set_managed(determinantField, true);
	cmzn_field_set_name(determinantField, "determinant");

	cmzn_field_id eigenvaluesField = cmzn_fieldmodule_create_field_eigenvalues(fieldmodule, matrixField);
	EXPECT_NE(static_cast<cmzn_field *>(0), eigenvaluesField);
	cmzn_field_set_managed(eigenvaluesField, true);
	cmzn_field_set_name(eigenvaluesField, "eigenvalues");

	cmzn_field_id eigenvectorsField = cmzn_fieldmodule_create_field_eigenvectors(
		fieldmodule, eigenvaluesField);
	EXPECT_NE(static_cast<cmzn_field *>(0), eigenvectorsField);
	cmzn_field_set_managed(eigenvectorsField, true);
	cmzn_field_set_name(eigenvectorsField, "eigenvectors");

	double values9_2[9] = {9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 2.0};
	cmzn_field_id matrixField2 = cmzn_fieldmodule_create_field_constant(
		fieldmodule, 9, values9_2);
	EXPECT_NE(static_cast<cmzn_field *>(0), matrixField2);
	cmzn_field_set_name(matrixField2, "matrix2");

	cmzn_field_id matrixMultiplyField = cmzn_fieldmodule_create_field_matrix_multiply(
		fieldmodule, 3, matrixField, matrixField2);
	EXPECT_NE(static_cast<cmzn_field *>(0), matrixMultiplyField);
	cmzn_field_set_managed(matrixMultiplyField, true);
	cmzn_field_set_name(matrixMultiplyField, "matrixMultiply");

	double values3_2[3] = {3.0, 2.0, 1.0};
	cmzn_field_id secondConstantField = cmzn_fieldmodule_create_field_constant(
		fieldmodule, 3, values3_2);
	EXPECT_NE(static_cast<cmzn_field *>(0), secondConstantField);
	cmzn_field_set_name(secondConstantField, "secondConstant");

	cmzn_field_id fieldArray[2] = {constantField, secondConstantField};

	cmzn_field_id crossProductField = cmzn_fieldmodule_create_field_cross_product(
		fieldmodule, 2, fieldArray);
	EXPECT_NE(static_cast<cmzn_field *>(0), crossProductField);
	cmzn_field_set_managed(crossProductField, true);
	cmzn_field_set_name(crossProductField, "crossProduct");

	cmzn_timekeepermodule_id tm = cmzn_context_get_timekeepermodule(context);
	cmzn_timekeeper_id timekeeper = cmzn_timekeepermodule_get_default_timekeeper(tm);
	cmzn_field_id timeValueField = cmzn_fieldmodule_create_field_time_value(
		fieldmodule, timekeeper);
	EXPECT_NE(static_cast<cmzn_field *>(0), timeValueField);
	cmzn_field_set_managed(timeValueField, true);
	cmzn_field_set_name(timeValueField, "timeValue");
	cmzn_timekeepermodule_destroy(&tm);
	cmzn_timekeeper_destroy(&timekeeper);

	cmzn_field_id timeLookupField = cmzn_fieldmodule_create_field_time_lookup(
		fieldmodule, coordinatesField, timeValueField);
	EXPECT_NE(static_cast<cmzn_field *>(0), timeLookupField);
	cmzn_field_set_managed(timeLookupField, true);
	cmzn_field_set_name(timeLookupField, "timeLookup");

	cmzn_field_id transposeField = cmzn_fieldmodule_create_field_transpose(
		fieldmodule, 3, matrixField);
	EXPECT_NE(static_cast<cmzn_field *>(0), transposeField);
	cmzn_field_set_managed(transposeField, true);
	cmzn_field_set_name(transposeField, "transpose");

	cmzn_field_id nodeValueField = cmzn_fieldmodule_create_field_node_value(
		fieldmodule, coordinatesField, CMZN_NODE_VALUE_LABEL_D_DS1, 2);
	EXPECT_NE(static_cast<cmzn_field *>(0), nodeValueField);
	cmzn_field_set_managed(nodeValueField, true);
	cmzn_field_set_name(nodeValueField, "nodeValue");

	cmzn_field_id isOnFaceField = cmzn_fieldmodule_create_field_is_on_face(
		fieldmodule, CMZN_ELEMENT_FACE_TYPE_XI1_0);
	EXPECT_NE(static_cast<cmzn_field *>(0), isOnFaceField);
	cmzn_field_set_managed(isOnFaceField, true);
	cmzn_field_set_name(isOnFaceField, "isOnFace");

	cmzn_mesh_id mesh3d = cmzn_fieldmodule_find_mesh_by_dimension(fieldmodule, 3);
	cmzn_mesh_id mesh2d = cmzn_fieldmodule_find_mesh_by_dimension(fieldmodule, 2);
	cmzn_field_id triangleField = cmzn_fieldmodule_find_field_by_name(fieldmodule, "triangle");
	cmzn_field_group_id triangleGroup = cmzn_field_cast_group(triangleField);
	cmzn_field_element_group_id triangleElementGroup2d = cmzn_field_group_get_field_element_group(triangleGroup, mesh2d);
	EXPECT_NE(static_cast<cmzn_field_element_group *>(0), triangleElementGroup2d);
	cmzn_mesh_group_id triangleMeshGroup2d = cmzn_field_element_group_get_mesh_group(triangleElementGroup2d);
	EXPECT_EQ(1, cmzn_mesh_get_size(cmzn_mesh_group_base_cast(triangleMeshGroup2d)));
	cmzn_nodeset_id nodes = cmzn_fieldmodule_find_nodeset_by_field_domain_type(fieldmodule, CMZN_FIELD_DOMAIN_TYPE_NODES);
	cmzn_field_node_group_id triangleNodeGroup = cmzn_field_group_get_field_node_group(triangleGroup, nodes);
	EXPECT_NE(static_cast<cmzn_field_node_group *>(0), triangleNodeGroup);
	cmzn_nodeset_group_id triangleNodesetGroup = cmzn_field_node_group_get_nodeset_group(triangleNodeGroup);
	EXPECT_EQ(3, cmzn_nodeset_get_size(cmzn_nodeset_group_base_cast(triangleNodesetGroup)));

	cmzn_field_id storedMeshLocationField = cmzn_fieldmodule_create_field_stored_mesh_location(
		fieldmodule, mesh2d);
	EXPECT_NE(static_cast<cmzn_field *>(0), storedMeshLocationField);
	cmzn_field_set_managed(storedMeshLocationField, true);
	cmzn_field_set_name(storedMeshLocationField, "storedMeshLocation");

	cmzn_field_id findMeshLocationField = cmzn_fieldmodule_create_field_find_mesh_location(
		fieldmodule, coordinatesField, coordinatesField, mesh3d);
	EXPECT_NE(static_cast<cmzn_field *>(0), findMeshLocationField);
	cmzn_field_set_managed(findMeshLocationField, true);
	cmzn_field_set_name(findMeshLocationField, "findMeshLocation");
	cmzn_field_find_mesh_location_id findMeshLocation = cmzn_field_cast_find_mesh_location(findMeshLocationField);
	EXPECT_NE(static_cast<cmzn_field_find_mesh_location *>(0), findMeshLocation);
	EXPECT_EQ(CMZN_OK, cmzn_field_find_mesh_location_set_search_mesh(findMeshLocation,
		cmzn_mesh_group_base_cast(triangleMeshGroup2d)));
	EXPECT_EQ(CMZN_OK, cmzn_field_find_mesh_location_set_search_mode(findMeshLocation,
		CMZN_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_NEAREST));
	cmzn_field_find_mesh_location_destroy(&findMeshLocation);
	cmzn_field_destroy(&findMeshLocationField);

	cmzn_field_id meshIntegralField = cmzn_fieldmodule_create_field_mesh_integral(
		fieldmodule, constantField, coordinatesField, cmzn_mesh_group_base_cast(triangleMeshGroup2d));
	EXPECT_NE(static_cast<cmzn_field *>(0), meshIntegralField);
	cmzn_field_set_managed(meshIntegralField, true);
	cmzn_field_set_name(meshIntegralField, "meshIntegral");
	cmzn_field_mesh_integral_id meshIntegral = cmzn_field_cast_mesh_integral(meshIntegralField);
	EXPECT_NE(static_cast<cmzn_field_mesh_integral *>(0), meshIntegral);
	EXPECT_EQ(CMZN_OK, cmzn_field_mesh_integral_set_element_quadrature_rule(meshIntegral,
		CMZN_ELEMENT_QUADRATURE_RULE_GAUSSIAN));
	const int numbersOfPoints1[2] = { 2, 4 };
	EXPECT_EQ(CMZN_OK, cmzn_field_mesh_integral_set_numbers_of_points(
		meshIntegral, 2, numbersOfPoints1));
	cmzn_field_mesh_integral_destroy(&meshIntegral);
	cmzn_field_destroy(&meshIntegralField);

	cmzn_field_id meshIntegralSquaresField = cmzn_fieldmodule_create_field_mesh_integral_squares(
		fieldmodule, constantField, coordinatesField, mesh2d);
	EXPECT_NE(static_cast<cmzn_field *>(0), meshIntegralSquaresField);
	cmzn_field_set_managed(meshIntegralSquaresField, true);
	cmzn_field_set_name(meshIntegralSquaresField, "meshIntegralSquares");
	// can only cast to mesh_integral type
	cmzn_field_mesh_integral_id meshIntegralSquares = cmzn_field_cast_mesh_integral(meshIntegralSquaresField);
	EXPECT_NE(static_cast<cmzn_field_mesh_integral *>(0), meshIntegralSquares);
	EXPECT_EQ(CMZN_OK, cmzn_field_mesh_integral_set_element_quadrature_rule(meshIntegralSquares,
		CMZN_ELEMENT_QUADRATURE_RULE_MIDPOINT));
	const int numbersOfPoints2[2] = { 3, 7 };
	EXPECT_EQ(CMZN_OK, cmzn_field_mesh_integral_set_numbers_of_points(
		meshIntegralSquares, 2, numbersOfPoints2));
	cmzn_field_mesh_integral_destroy(&meshIntegralSquares);
	cmzn_field_destroy(&meshIntegralSquaresField);

	const char *nodesetOperatorNames[6] = { "Maximum", "Mean", "MeanSquares", "Minimum", "Sum", "SumSquares" };
	for (int i = 0; i < 6; ++i)
	{
		std::string fieldName("nodeset");
		fieldName += nodesetOperatorNames[i];
		cmzn_field_id nodesetOperatorField =
			(i == 0) ? cmzn_fieldmodule_create_field_nodeset_maximum(fieldmodule, coordinatesField, nodes) :
			(i == 1) ? cmzn_fieldmodule_create_field_nodeset_mean(fieldmodule, coordinatesField, nodes) :
			(i == 2) ? cmzn_fieldmodule_create_field_nodeset_mean_squares(fieldmodule, coordinatesField, nodes) :
			(i == 3) ? cmzn_fieldmodule_create_field_nodeset_minimum(fieldmodule, coordinatesField, nodes) :
			(i == 4) ? cmzn_fieldmodule_create_field_nodeset_sum(fieldmodule, coordinatesField, cmzn_nodeset_group_base_cast(triangleNodesetGroup)) :
			(i == 5) ? cmzn_fieldmodule_create_field_nodeset_sum_squares(fieldmodule, coordinatesField, nodes) :
			static_cast<cmzn_field *>(0);
		EXPECT_NE(static_cast<cmzn_field *>(0), nodesetOperatorField);
		cmzn_field_set_managed(nodesetOperatorField, true);
		cmzn_field_set_name(nodesetOperatorField, fieldName.c_str());
		if (i == 4)
		{
			cmzn_field_nodeset_operator_id nodesetSum = cmzn_field_cast_nodeset_operator(nodesetOperatorField);
			EXPECT_NE(static_cast<cmzn_field_nodeset_operator *>(0), nodesetSum);
			EXPECT_EQ(CMZN_OK, cmzn_field_nodeset_operator_set_element_map_field(nodesetSum, storedMeshLocationField));
			cmzn_field_nodeset_operator_destroy(&nodesetSum);
		}
		cmzn_field_destroy(&nodesetOperatorField);
	}

	cmzn_nodeset_group_destroy(&triangleNodesetGroup);
	cmzn_field_node_group_destroy(&triangleNodeGroup);
	cmzn_nodeset_destroy(&nodes);
	cmzn_mesh_group_destroy(&triangleMeshGroup2d);
	cmzn_field_element_group_destroy(&triangleElementGroup2d);
	cmzn_field_group_destroy(&triangleGroup);
	cmzn_field_destroy(&triangleField);
	cmzn_mesh_destroy(&mesh2d);
	cmzn_mesh_destroy(&mesh3d);

	char *description_string = cmzn_fieldmodule_write_description(fieldmodule);
	EXPECT_NE(static_cast<char *>(0), description_string);
	//printf("%s", description_string);

	cmzn_region_id region = cmzn_region_create_child(root_region, "test");
    TestDescriptionOutput(region, description_string);
	cmzn_region_destroy(&region);

	cmzn_deallocate(description_string);

	cmzn_field_destroy(&storedMeshLocationField);
	cmzn_field_destroy(&isOnFaceField);
	cmzn_field_destroy(&nodeValueField);
	cmzn_field_destroy(&transposeField);
	cmzn_field_destroy(&timeLookupField);
	cmzn_field_destroy(&timeValueField);
	cmzn_field_destroy(&crossProductField);
	cmzn_field_destroy(&secondConstantField);
	cmzn_field_destroy(&matrixMultiplyField);
	cmzn_field_destroy(&matrixField2);
	cmzn_field_destroy(&eigenvectorsField);
	cmzn_field_destroy(&eigenvaluesField);
	cmzn_field_destroy(&determinantField);
	cmzn_field_destroy(&matrixField);
	cmzn_field_destroy(&coordinateTransformationField);
	cmzn_field_destroy(&derivativeField);
	cmzn_field_destroy(&edgeDiscontinuityField);
	cmzn_field_destroy(&stringConstantField);
	cmzn_field_destroy(&constantField);
	cmzn_field_destroy(&concatenateField);
	cmzn_field_destroy(&thirdComponentField);
	cmzn_field_destroy(&secondComponentField);
	cmzn_field_destroy(&firstComponentField);
	cmzn_field_destroy(&componentField);
	cmzn_field_destroy(&temperatureField);
	cmzn_field_destroy(&coordinatesField);
	cmzn_fieldmodule_destroy(&fieldmodule);

	cmzn_region_destroy(&tetrahedron_region);
	cmzn_region_destroy(&root_region);
	cmzn_context_destroy(&context);
}

TEST(fieldmodule_description, read)
{
    cmzn_context_id context = cmzn_context_create("test");
    cmzn_region_id root_region = cmzn_context_get_default_region(context);

    std::string stringBuffer = fileContents("fieldmodule/fieldmodule_description.json");

    TestDescriptionOutput(root_region, stringBuffer.c_str());

    cmzn_region_destroy(&root_region);
    cmzn_context_destroy(&context);
}

// Test serialisation of field with NOT_APPLICABLE coordinate system
TEST(Fieldmodule_description, writeFieldCoordinateSystemTypeNotApplicable)
{
	ZincTestSetupCpp zinc;

	FieldFiniteElement field = zinc.fm.createFieldFiniteElement(1);
	EXPECT_EQ(Field::COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN, field.getCoordinateSystemType());
	EXPECT_EQ(RESULT_OK, field.setCoordinateSystemType(Field::COORDINATE_SYSTEM_TYPE_NOT_APPLICABLE));
	EXPECT_EQ(Field::COORDINATE_SYSTEM_TYPE_NOT_APPLICABLE, field.getCoordinateSystemType());

	char *description = zinc.fm.writeDescription();
	EXPECT_NE(nullptr, description);
	EXPECT_EQ(RESULT_OK, zinc.fm.readDescription(description));
	cmzn_deallocate(description);
}
