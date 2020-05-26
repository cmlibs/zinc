/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <opencmiss/zinc/element.hpp>
#include <opencmiss/zinc/field.hpp>
#include <opencmiss/zinc/fieldcache.hpp>
#include <opencmiss/zinc/fieldconstant.hpp>
#include <opencmiss/zinc/fieldgroup.hpp>
#include <opencmiss/zinc/fieldmeshoperators.hpp>
#include "opencmiss/zinc/fieldsubobjectgroup.hpp"
#include <opencmiss/zinc/node.hpp>
#include "zinctestsetupcpp.hpp"

#include "test_resources.h"


TEST(ZincElementbasis, element_bases_3d)
{
	ZincTestSetupCpp zinc;

	EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDMODULE_EX2_ELEMENT_BASES_3D_RESOURCE)));

	const int total_element_count = 12;
	const Element::ShapeType cube_face_shapes[6] = {
		Element::SHAPE_TYPE_SQUARE, Element::SHAPE_TYPE_SQUARE, Element::SHAPE_TYPE_SQUARE,
		Element::SHAPE_TYPE_SQUARE, Element::SHAPE_TYPE_SQUARE, Element::SHAPE_TYPE_SQUARE } ;
	const Element::ShapeType wedge12_face_shapes[5] = {
		Element::SHAPE_TYPE_SQUARE, Element::SHAPE_TYPE_SQUARE, Element::SHAPE_TYPE_SQUARE,
		Element::SHAPE_TYPE_TRIANGLE, Element::SHAPE_TYPE_TRIANGLE };
	const Element::ShapeType wedge13_face_shapes[5] = {
		Element::SHAPE_TYPE_SQUARE, Element::SHAPE_TYPE_TRIANGLE, Element::SHAPE_TYPE_TRIANGLE,
		Element::SHAPE_TYPE_SQUARE, Element::SHAPE_TYPE_SQUARE };
	const Element::ShapeType wedge23_face_shapes[5] = {
		Element::SHAPE_TYPE_TRIANGLE, Element::SHAPE_TYPE_TRIANGLE,
		Element::SHAPE_TYPE_SQUARE, Element::SHAPE_TYPE_SQUARE, Element::SHAPE_TYPE_SQUARE };
	const Element::ShapeType tetrahedron_face_shapes[4] = {
		Element::SHAPE_TYPE_TRIANGLE, Element::SHAPE_TYPE_TRIANGLE,
		Element::SHAPE_TYPE_TRIANGLE, Element::SHAPE_TYPE_TRIANGLE };
	struct
	{
		const char *basis_name;
		const int face_count;
		const Element::ShapeType *face_shapes;
		const int line_count;
		const int node_count;
		const double volume;
		const double surface_area;
	} basis_info[total_element_count] =
	{
		{ "cubic_cubic_cubic",          6, cube_face_shapes,       12,  8, 0.49999999999999917,  4.1784784082885480  },
		{ "cubic_cubic_linear",         6, cube_face_shapes,       12,  8, 0.49999999999999956,  4.1046338562917315  },
		{ "cubic_linear_cubic",         6, cube_face_shapes,       12,  8, 0.50000000000000067,  4.2958856313845120  },
		{ "cubic_lsimplex_lsimplex",    5, wedge23_face_shapes,     9,  6, 0.25722675528275141,  3.2056678009417579  },
		{ "linear_cubic_cubic",         6, cube_face_shapes,       12,  8, 0.64343999856732226,  5.0045880438338406  },
		{ "linear_lsimplex_lsimplex",   5, wedge23_face_shapes,     9,  6, 0.24999999999999986,  3.1180339887498953  },
		{ "lsimplex_cubic_lsimplex",    5, wedge13_face_shapes,     9,  6, 0.23053849440649571,  2.7719159844930603  },
		{ "lsimplex_linear_lsimplex",   5, wedge13_face_shapes,     9,  6, 0.24999999999999986,  3.1180339887498953  },
		{ "lsimplex_lsimplex_cubic",    5, wedge12_face_shapes,     9,  6, 0.17936266345881541,  2.2540138554161726  },
		{ "lsimplex_lsimplex_linear",   5, wedge12_face_shapes,     9,  6, 0.24999999999999986,  2.7071067811865475  },
		{ "lsimplex_lsimplex_lsimplex", 4, tetrahedron_face_shapes, 6,  4, 0.020833333333333245, 0.57279395107029596 },
		{ "qsimplex_qsimplex_qsimplex", 4, tetrahedron_face_shapes, 6, 10, 0.13599999999999951,  2.2723185285043952  }
	};
	int total_face_count = 0;
	int total_line_count = 0;
	int total_node_count = 0;
	for (int i = 0; i < total_element_count; ++i)
	{
		total_face_count += basis_info[i].face_count;
		total_line_count += basis_info[i].line_count;
		total_node_count += basis_info[i].node_count;
	}
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	Mesh mesh1d = zinc.fm.findMeshByDimension(1);
	Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_EQ(total_element_count, mesh3d.getSize());
	EXPECT_EQ(total_node_count, nodes.getSize());
	EXPECT_EQ(0, mesh2d.getSize());
	EXPECT_EQ(0, mesh1d.getSize());
	EXPECT_EQ(RESULT_OK, zinc.fm.defineAllFaces());
	EXPECT_EQ(total_face_count, mesh2d.getSize());
	EXPECT_EQ(total_line_count, mesh1d.getSize());

	zinc.fm.beginChange();
	Fieldcache fieldcache = zinc.fm.createFieldcache();
	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	double constant_one = 1.0;
	FieldConstant one = zinc.fm.createFieldConstant(1, &constant_one);
	EXPECT_TRUE(one.isValid());
	int four = 4;
	double surface_area, volume;
	const double TOL = 1.0E-8;
	double values[12][2];
	for (int i = 0; i < total_element_count; ++i)
	{
		const char *basis_name = basis_info[i].basis_name;
		//std::cerr << i << ". " << basis_name << "\n";
		FieldGroup group = zinc.fm.findFieldByName(basis_name).castGroup();
		EXPECT_TRUE(group.isValid());
		FieldElementGroup elementGroup = group.getFieldElementGroup(mesh3d);
		EXPECT_TRUE(elementGroup.isValid());
		MeshGroup elementMeshGroup = elementGroup.getMeshGroup();
		EXPECT_EQ(1, elementMeshGroup.getSize());
		// add elements to itself to add faces, lines, nodes to group etc.
		EXPECT_EQ(RESULT_OK, group.setSubelementHandlingMode(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL));
		EXPECT_EQ(RESULT_OK, elementMeshGroup.addElementsConditional(elementGroup));
		FieldElementGroup faceGroup = group.getFieldElementGroup(mesh2d);
		EXPECT_TRUE(faceGroup.isValid());
		MeshGroup faceMeshGroup = faceGroup.getMeshGroup();
		EXPECT_EQ(basis_info[i].face_count, faceMeshGroup.getSize());
		// check faces have the expected shapes
		Elementiterator faceIter = faceMeshGroup.createElementiterator();
		for (int f = 0; f < basis_info[i].face_count; ++f)
		{
			Element face = faceIter.next();
			EXPECT_EQ(basis_info[i].face_shapes[f], face.getShapeType());
		}
		FieldElementGroup lineGroup = group.getFieldElementGroup(mesh1d);
		EXPECT_TRUE(lineGroup.isValid());
		MeshGroup lineMeshGroup = lineGroup.getMeshGroup();
		EXPECT_EQ(basis_info[i].line_count, lineMeshGroup.getSize());
		FieldNodeGroup nodeGroup = group.getFieldNodeGroup(nodes);
		EXPECT_TRUE(nodeGroup.isValid());
		NodesetGroup nodeNodesetGroup = nodeGroup.getNodesetGroup();
		EXPECT_EQ(basis_info[i].node_count, nodeNodesetGroup.getSize());
		FieldMeshIntegral volume_integral = zinc.fm.createFieldMeshIntegral(one, coordinates, elementMeshGroup);
		EXPECT_TRUE(volume_integral.isValid());
		EXPECT_EQ(RESULT_OK, volume_integral.setNumbersOfPoints(1, &four));
		FieldMeshIntegral surface_area_integral = zinc.fm.createFieldMeshIntegral(one, coordinates, faceMeshGroup);
		EXPECT_EQ(RESULT_OK, surface_area_integral.setNumbersOfPoints(1, &four));
		EXPECT_TRUE(surface_area_integral.isValid());
		EXPECT_EQ(RESULT_OK, volume_integral.evaluateReal(fieldcache, 1, &volume));
		EXPECT_EQ(RESULT_OK, surface_area_integral.evaluateReal(fieldcache, 1, &surface_area));
		values[i][0] = volume;
		values[i][1] = surface_area;
		EXPECT_NEAR(basis_info[i].volume, volume, TOL);
		EXPECT_NEAR(basis_info[i].surface_area, surface_area, TOL);
	}
	zinc.fm.endChange();
}
