/*
 * Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>
#include <cmath>

#include <cmlibs/zinc/changemanager.hpp>
#include <cmlibs/zinc/core.h>
#include <cmlibs/zinc/element.hpp>
#include <cmlibs/zinc/field.hpp>
#include <cmlibs/zinc/fieldarithmeticoperators.hpp>
#include <cmlibs/zinc/fieldcache.hpp>
#include <cmlibs/zinc/fieldconstant.hpp>
#include <cmlibs/zinc/fieldfiniteelement.hpp>
#include <cmlibs/zinc/fieldgroup.hpp>
#include <cmlibs/zinc/fieldlogicaloperators.hpp>
#include <cmlibs/zinc/fieldmeshoperators.hpp>
#include <cmlibs/zinc/fieldmodule.hpp>
#include <cmlibs/zinc/region.hpp>
#include <cmlibs/zinc/result.hpp>
#include <cmlibs/zinc/streamregion.hpp>

#include "utilities/zinctestsetupcpp.hpp"

#include "test_resources.h"

namespace {
ManageOutputFolder manageOutputFolderFieldML("/fieldml");
}

namespace {

	const double PI = 3.14159265358979323846;

}

namespace {

void check_cube_model(Fieldmodule& fm)
{
    int result;
    Field coordinates = fm.findFieldByName("coordinates");
    EXPECT_TRUE(coordinates.isValid());
    EXPECT_EQ(3, coordinates.getNumberOfComponents());
    EXPECT_TRUE(coordinates.isTypeCoordinate());

    Field pressure = fm.findFieldByName("pressure");
    EXPECT_TRUE(pressure.isValid());
    EXPECT_EQ(1, pressure.getNumberOfComponents());
    EXPECT_FALSE(pressure.isTypeCoordinate());

    EXPECT_EQ(OK, result = fm.defineAllFaces());
    Mesh mesh3d = fm.findMeshByDimension(3);
    EXPECT_EQ(1, mesh3d.getSize());
    Mesh mesh2d = fm.findMeshByDimension(2);
    EXPECT_EQ(6, mesh2d.getSize());
    Mesh mesh1d = fm.findMeshByDimension(1);
    EXPECT_EQ(12, mesh1d.getSize());
    Nodeset nodes = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
    EXPECT_EQ(8, nodes.getSize());
    Element element = mesh3d.findElementByIdentifier(1);
    EXPECT_TRUE(element.isValid());
    EXPECT_EQ(Element::SHAPE_TYPE_CUBE, element.getShapeType());

    const double valueOne = 1.0;
    Field one = fm.createFieldConstant(1, &valueOne);
    FieldMeshIntegral volume = fm.createFieldMeshIntegral(one, coordinates, mesh3d);
    const int numberOfPoints = 2;
    EXPECT_EQ(OK, result = volume.setNumbersOfPoints(1, &numberOfPoints));
    FieldMeshIntegral surfacePressureIntegral = fm.createFieldMeshIntegral(pressure, coordinates, mesh2d);
    EXPECT_EQ(OK, result = surfacePressureIntegral.setNumbersOfPoints(1, &numberOfPoints));

    Fieldcache cache = fm.createFieldcache();
    double outVolume;
    EXPECT_EQ(OK, result = volume.evaluateReal(cache, 1, &outVolume));
    ASSERT_DOUBLE_EQ(1.0, outVolume);
    double outSurfacePressureIntegral;
    EXPECT_EQ(OK, result = surfacePressureIntegral.evaluateReal(cache, 1, &outSurfacePressureIntegral));
    ASSERT_DOUBLE_EQ(540000.0, outSurfacePressureIntegral);
}

}

// Test I/O of unit cube model using EX2 format
// cube model defines a 3-D RC coordinates field and 1-D pressure field
// using the same trilinear Lagrange scalar template.
// field dofs and mesh nodes connectivity are inline text in the fieldml document
TEST(ZincRegion, ex2_cube)
{
    ZincTestSetupCpp zinc;
    int result;

    EXPECT_EQ(OK, result = zinc.root_region.readFile(
        resourcePath("cube.ex2").c_str()));
    check_cube_model(zinc.fm);

    std::string outFile = manageOutputFolderFieldML.getPath("/cube.ex2");
    // test writing and re-reading into different region
    EXPECT_EQ(OK, result = zinc.root_region.writeFile(outFile.c_str()));
    Region testRegion = zinc.root_region.createChild("test");
    EXPECT_EQ(OK, result = testRegion.readFile(outFile.c_str()));
    Fieldmodule testFm = testRegion.getFieldmodule();
    check_cube_model(testFm);
}

// Test I/O of unit cube model using FieldML format
// cube model defines a 3-D RC coordinates field and 1-D pressure field
// using the same trilinear Lagrange scalar template.
// field dofs and mesh nodes connectivity are inline text in the fieldml document
TEST(ZincRegion, fieldml_cube)
{
    ZincTestSetupCpp zinc;
    int result;

    // initial input file is in legacy FieldML format not using element field templates
    EXPECT_EQ(OK, result = zinc.root_region.readFile(
        resourcePath("fieldio/cube.fieldml").c_str()));
    check_cube_model(zinc.fm);

    std::string outFile = manageOutputFolderFieldML.getPath("/cube.fieldml");
    // test writing and re-reading into different region
    EXPECT_EQ(OK, result = zinc.root_region.writeFile(outFile.c_str()));
    Region testRegion = zinc.root_region.createChild("test");
    EXPECT_EQ(OK, result = testRegion.readFile(outFile.c_str()));
    Fieldmodule testFm = testRegion.getFieldmodule();
    check_cube_model(testFm);

    std::string outFileNonCoordinate = manageOutputFolderFieldML.getPath("/cube_noncoordinate.fieldml");
    // test having a non-coordinate multi-component field
    Field coordinates = testFm.findFieldByName("coordinates");
    EXPECT_TRUE(coordinates.isTypeCoordinate());
    EXPECT_EQ(OK, result = coordinates.setTypeCoordinate(false));
    EXPECT_EQ(OK, result = testRegion.writeFile(outFileNonCoordinate.c_str()));

    Region testRegion2 = zinc.root_region.createChild("test2");
    EXPECT_EQ(OK, result = testRegion2.readFile(outFileNonCoordinate.c_str()));
    Fieldmodule testFm2 = testRegion2.getFieldmodule();
    Field coordinates2 = testFm2.findFieldByName("coordinates");
    EXPECT_FALSE(coordinates2.isTypeCoordinate());
    EXPECT_EQ(OK, result = coordinates2.setTypeCoordinate(true));
    check_cube_model(testFm2);
}

// Also reads cube model, but tries to read it as EX format which should fail
TEST(ZincStreaminformationRegion, fileFormat)
{
    ZincTestSetupCpp zinc;
    int result;

    StreaminformationRegion streamInfo = zinc.root_region.createStreaminformationRegion();
    EXPECT_TRUE(streamInfo.isValid());
    StreamresourceFile fileResource = streamInfo.createStreamresourceFile(
        resourcePath("fieldio/cube.fieldml").c_str());
    EXPECT_TRUE(fileResource.isValid());
    StreaminformationRegion::FileFormat fileFormat = streamInfo.getFileFormat();
    EXPECT_EQ(StreaminformationRegion::FILE_FORMAT_AUTOMATIC, fileFormat);
    EXPECT_EQ(OK, result = streamInfo.setFileFormat(StreaminformationRegion::FILE_FORMAT_EX));
    EXPECT_EQ(StreaminformationRegion::FILE_FORMAT_EX, fileFormat = streamInfo.getFileFormat());
    result = zinc.root_region.read(streamInfo);
    EXPECT_EQ(ERROR_GENERAL, result); // not in EX format
    EXPECT_EQ(OK, result = streamInfo.setFileFormat(StreaminformationRegion::FILE_FORMAT_FIELDML));
    EXPECT_EQ(StreaminformationRegion::FILE_FORMAT_FIELDML, fileFormat = streamInfo.getFileFormat());
    result = zinc.root_region.read(streamInfo);
    EXPECT_EQ(OK, result); // in FieldML format
    check_cube_model(zinc.fm);
}

namespace {

void check_tetmesh_model(Fieldmodule& fm)
{
    int result;
    Field coordinates = fm.findFieldByName("coordinates");
    EXPECT_TRUE(coordinates.isValid());
    EXPECT_EQ(3, coordinates.getNumberOfComponents());
    EXPECT_TRUE(coordinates.isTypeCoordinate());

    EXPECT_EQ(OK, result = fm.defineAllFaces());
    Mesh mesh3d = fm.findMeshByDimension(3);
    const int elementsCount = mesh3d.getSize();
    EXPECT_EQ(102, elementsCount);
    Mesh mesh2d = fm.findMeshByDimension(2);
    EXPECT_EQ(232, mesh2d.getSize());
    Mesh mesh1d = fm.findMeshByDimension(1);
    EXPECT_EQ(167, mesh1d.getSize());
    Nodeset nodes = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
    EXPECT_EQ(38, nodes.getSize());
    for (int e = 1; e <= elementsCount; ++e)
    {
        Element element = mesh3d.findElementByIdentifier(e);
        EXPECT_TRUE(element.isValid());
        Element::ShapeType shapeType = element.getShapeType();
        EXPECT_EQ(Element::SHAPE_TYPE_TETRAHEDRON, shapeType);
    }

    const double valueOne = 1.0;
    Field one = fm.createFieldConstant(1, &valueOne);
    FieldMeshIntegral volume = fm.createFieldMeshIntegral(one, coordinates, mesh3d);
    EXPECT_TRUE(volume.isValid());

    FieldGroup exteriorFacesGroup = fm.createFieldGroup();
    EXPECT_TRUE(exteriorFacesGroup.isValid());
    EXPECT_EQ(OK, result = exteriorFacesGroup.setManaged(true));
    MeshGroup exteriorFacesMeshGroup = exteriorFacesGroup.createMeshGroup(mesh2d);
    EXPECT_TRUE(exteriorFacesMeshGroup.isValid());
    FieldIsExterior isExterior = fm.createFieldIsExterior();
    EXPECT_TRUE(isExterior.isValid());
    exteriorFacesMeshGroup.addElementsConditional(isExterior);
    EXPECT_EQ(56, exteriorFacesMeshGroup.getSize());
    FieldMeshIntegral surfaceArea = fm.createFieldMeshIntegral(one, coordinates, exteriorFacesMeshGroup);
    EXPECT_TRUE(surfaceArea.isValid());

    Fieldcache cache = fm.createFieldcache();
    double outVolume;
    EXPECT_EQ(OK, result = volume.evaluateReal(cache, 1, &outVolume));
    //ASSERT_DOUBLE_EQ(0.41723178864303812, outVolume);
    EXPECT_NEAR(0.41723178864303812, outVolume, 0.5E-7);
    double outSurfaceArea;
    EXPECT_EQ(OK, result = surfaceArea.evaluateReal(cache, 1, &outSurfaceArea));
    //ASSERT_DOUBLE_EQ(2.7717561493468423, outSurfaceArea);
    EXPECT_NEAR(2.7717561493468423, outSurfaceArea, 1.0E-7);
}

}

// Test I/O of tetmesh model using EX2 format
// tetmesh model defines a 3-D RC coordinates field over a tetrahedral
// mesh in approximate unit sphere shape with trilinearSimplex basis/
// node coordinates and connectivity are read from separate files
TEST(ZincRegion, ex2_tetmesh)
{
    ZincTestSetupCpp zinc;
    int result;

    EXPECT_EQ(OK, result = zinc.root_region.readFile(
        resourcePath("fieldio/tetmesh.ex2").c_str()));
    check_tetmesh_model(zinc.fm);

    std::string outFile = manageOutputFolderFieldML.getPath("/tetmesh.ex2");
    // test writing and re-reading into different region
    EXPECT_EQ(OK, result = zinc.root_region.writeFile(outFile.c_str()));
    Region testRegion = zinc.root_region.createChild("test");
    EXPECT_EQ(OK, result = testRegion.readFile(outFile.c_str()));
    Fieldmodule testFm = testRegion.getFieldmodule();
    check_tetmesh_model(testFm);
}

// Test I/O of tetmesh model using FieldML format
// tetmesh model defines a 3-D RC coordinates field over a tetrahedral
// mesh in approximate unit sphere shape with trilinearSimplex basis/
// node coordinates and connectivity are read from separate files
TEST(ZincRegion, fieldml_tetmesh)
{
    ZincTestSetupCpp zinc;
    int result;

    std::string inFile = resourcePath("fieldio/tetmesh.fieldml");
    EXPECT_EQ(OK, result = zinc.root_region.readFile(inFile.c_str()));
    check_tetmesh_model(zinc.fm);

    // check can't merge cube model since it redefines element 1 shape
    result = zinc.root_region.readFile(
        resourcePath("fieldio/cube.fieldml").c_str());
    EXPECT_EQ(ERROR_INCOMPATIBLE_DATA, result);

    std::string outFile = manageOutputFolderFieldML.getPath("/tetmesh.fieldml");
    // test writing and re-reading into different region
    EXPECT_EQ(OK, result = zinc.root_region.writeFile(outFile.c_str()));
    Region testRegion = zinc.root_region.createChild("test");
    EXPECT_EQ(OK, result = testRegion.readFile(outFile.c_str()));
    Fieldmodule testFm = testRegion.getFieldmodule();
    check_tetmesh_model(testFm);
}

namespace {

void check_wheel_model(Fieldmodule& fm)
{
    int result;
    Field coordinates = fm.findFieldByName("coordinates");
    EXPECT_TRUE(coordinates.isValid());
    EXPECT_EQ(3, coordinates.getNumberOfComponents());
    EXPECT_TRUE(coordinates.isTypeCoordinate());

    EXPECT_EQ(OK, result = fm.defineAllFaces());
    Mesh mesh3d = fm.findMeshByDimension(3);
    const int elementsCount = mesh3d.getSize();
    EXPECT_EQ(12, elementsCount);
    Mesh mesh2d = fm.findMeshByDimension(2);
    EXPECT_EQ(48, mesh2d.getSize());
    Mesh mesh1d = fm.findMeshByDimension(1);
    EXPECT_EQ(61, mesh1d.getSize());
    Nodeset nodes = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
    EXPECT_EQ(129, nodes.getSize());
    for (int e = 1; e <= elementsCount; ++e)
    {
        Element element = mesh3d.findElementByIdentifier(e);
        EXPECT_TRUE(element.isValid());
        Element::ShapeType shapeType = element.getShapeType();
        if (e <= 6)
            EXPECT_EQ(Element::SHAPE_TYPE_WEDGE12, shapeType);
        else
            EXPECT_EQ(Element::SHAPE_TYPE_CUBE, shapeType);
    }

    const double valueOne = 1.0;
    Field one = fm.createFieldConstant(1, &valueOne);
    FieldMeshIntegral volume = fm.createFieldMeshIntegral(one, coordinates, mesh3d);
    EXPECT_TRUE(volume.isValid());
    const int pointCount = 2;
    EXPECT_EQ(OK, result = volume.setNumbersOfPoints(1, &pointCount));

    FieldGroup exteriorFacesGroup = fm.createFieldGroup();
    EXPECT_TRUE(exteriorFacesGroup.isValid());
    EXPECT_EQ(OK, result = exteriorFacesGroup.setManaged(true));
    MeshGroup exteriorFacesMeshGroup = exteriorFacesGroup.createMeshGroup(mesh2d);
    EXPECT_TRUE(exteriorFacesMeshGroup.isValid());
    FieldIsExterior isExterior = fm.createFieldIsExterior();
    EXPECT_TRUE(isExterior.isValid());
    exteriorFacesMeshGroup.addElementsConditional(isExterior);
    EXPECT_EQ(30, exteriorFacesMeshGroup.getSize());
    FieldMeshIntegral surfaceArea = fm.createFieldMeshIntegral(one, coordinates, exteriorFacesMeshGroup);
    EXPECT_TRUE(surfaceArea.isValid());
    EXPECT_EQ(OK, result = surfaceArea.setNumbersOfPoints(1, &pointCount));

    Fieldcache cache = fm.createFieldcache();
    double outVolume;
    EXPECT_EQ(OK, result = volume.evaluateReal(cache, 1, &outVolume));
    EXPECT_NEAR(100.28718664065387, outVolume, 5.0E-5);
    double outSurfaceArea;
    EXPECT_EQ(OK, result = surfaceArea.evaluateReal(cache, 1, &outSurfaceArea));
    EXPECT_NEAR(150.53218306379620, outSurfaceArea, 1.0E-4);
}

}

// Test I/O of wheel model in EX2 format
// wheel_indirect model is the same as the wheel_direct model except that it
// uses a more efficient indirect element-to-function map
TEST(ZincRegion, ex2_wheel)
{
    ZincTestSetupCpp zinc;
    int result;
    EXPECT_EQ(OK, result = zinc.root_region.readFile(
        resourcePath("fieldio/wheel.ex2").c_str()));
    check_wheel_model(zinc.fm);

    std::string outFile = manageOutputFolderFieldML.getPath("/wheel.ex2");
    // test writing and re-reading into different region
    EXPECT_EQ(OK, result = zinc.root_region.writeFile(outFile.c_str()));
    Region testRegion = zinc.root_region.createChild("test");
    EXPECT_EQ(OK, result = testRegion.readFile(outFile.c_str()));
    Fieldmodule testFm = testRegion.getFieldmodule();
    check_wheel_model(testFm);
}

// test reading FieldML wheel model with direct element function map
// wheel_direct model defines a 3-D RC coordinates field over a wheel mesh
// consisting of 6 wedge elements in the centre, and 6 cube elements around
// them, all coordinates interpolated with triquadratic bases.
// This model tests having variant element shapes and a piecewise field
// template which directly maps element to function (basis + parameter map).
// It also reads shapeids, node coordinates and connectivity (for wedge and
// cube connectivity) from separate files, and the connectivity data uses
// dictionary of keys (DOK) format with key data in the first column of the
// same file.
TEST(ZincRegion, fieldml_wheel_direct)
{
    ZincTestSetupCpp zinc;
    int result;
    EXPECT_EQ(OK, result = zinc.root_region.readFile(
        resourcePath("fieldio/wheel_direct.fieldml").c_str()));
    check_wheel_model(zinc.fm);
}

// Test I/O of wheel model in FieldML format
// wheel_indirect model is the same as the wheel_direct model except that it
// uses a more efficient indirect element-to-function map
TEST(ZincRegion, fieldml_wheel_indirect)
{
    ZincTestSetupCpp zinc;
    int result;
    EXPECT_EQ(OK, result = zinc.root_region.readFile(
        resourcePath("fieldio/wheel_indirect.fieldml").c_str()));
    check_wheel_model(zinc.fm);

    std::string outFile = manageOutputFolderFieldML.getPath("/wheel_indirect.fieldml");
    // test writing and re-reading into different region
    EXPECT_EQ(OK, result = zinc.root_region.writeFile(outFile.c_str()));
    Region testRegion = zinc.root_region.createChild("test");
    EXPECT_EQ(OK, result = testRegion.readFile(outFile.c_str()));
    Fieldmodule testFm = testRegion.getFieldmodule();
    check_wheel_model(testFm);
}

namespace {

void create_mixed_template_squares(Fieldmodule& fm)
{
    ChangeManager<Fieldmodule> changeField(fm);
    int result;

    FieldFiniteElement coordinates = fm.createFieldFiniteElement(/*numberOfComponents*/2);
    EXPECT_TRUE(coordinates.isValid());
    EXPECT_EQ(RESULT_OK, result = coordinates.setName("coordinates"));
    EXPECT_EQ(RESULT_OK, result = coordinates.setTypeCoordinate(true));
    EXPECT_EQ(RESULT_OK, result = coordinates.setManaged(true));
    EXPECT_EQ(RESULT_OK, result = coordinates.setComponentName(1, "x"));
    EXPECT_EQ(RESULT_OK, result = coordinates.setComponentName(2, "y"));

    FieldFiniteElement pressure = fm.createFieldFiniteElement(/*numberOfComponents*/1);
    EXPECT_TRUE(pressure.isValid());
    EXPECT_EQ(RESULT_OK, result = pressure.setName("pressure"));
    EXPECT_EQ(RESULT_OK, result = pressure.setManaged(true));

    FieldFiniteElement temperature = fm.createFieldFiniteElement(/*numberOfComponents*/1);
    EXPECT_TRUE(temperature.isValid());
    EXPECT_EQ(RESULT_OK, result = temperature.setName("temperature"));
    EXPECT_EQ(RESULT_OK, result = temperature.setManaged(true));

    Nodeset nodeset = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
    EXPECT_TRUE(nodeset.isValid());

    Nodetemplate nodetemplate_cpt = nodeset.createNodetemplate();
    EXPECT_EQ(RESULT_OK, result = nodetemplate_cpt.defineField(coordinates));
    EXPECT_EQ(RESULT_OK, result = nodetemplate_cpt.defineField(pressure));
    EXPECT_EQ(RESULT_OK, result = nodetemplate_cpt.defineField(temperature));
    Nodetemplate nodetemplate_cp = nodeset.createNodetemplate();
    EXPECT_EQ(RESULT_OK, result = nodetemplate_cp.defineField(coordinates));
    EXPECT_EQ(RESULT_OK, result = nodetemplate_cp.defineField(pressure));
    Nodetemplate nodetemplate_ct = nodeset.createNodetemplate();
    EXPECT_EQ(RESULT_OK, result = nodetemplate_ct.defineField(coordinates));
    EXPECT_EQ(RESULT_OK, result = nodetemplate_ct.defineField(temperature));
    Nodetemplate nodetemplate_c = nodeset.createNodetemplate();
    EXPECT_EQ(RESULT_OK, result = nodetemplate_c.defineField(coordinates));
    Fieldcache fieldcache = fm.createFieldcache();
    for (int j = 0; j < 9; ++j)
        for (int i = 0; i < 9; ++i)
        {
            Node node;
            int identifier = j*9 + i + 1;
            const bool linearNode = (0 == (i % 2)) && (0 == (j % 2));
            bool hasPressure = (j < 7) && linearNode;
            bool hasTemperature = (j > 1);
            if (hasPressure)
            {
                if (hasTemperature)
                    node = nodeset.createNode(identifier, nodetemplate_cpt);
                else
                    node = nodeset.createNode(identifier, nodetemplate_cp);
            }
            else if (hasTemperature)
                node = nodeset.createNode(identifier, nodetemplate_ct);
            else
                node = nodeset.createNode(identifier, nodetemplate_c);
            EXPECT_EQ(RESULT_OK, fieldcache.setNode(node));
            double coordinatesValues[2] = { i*0.5, j*0.5 };
            EXPECT_EQ(RESULT_OK, coordinates.assignReal(fieldcache, 2, coordinatesValues));
            if (hasPressure)
            {
                double pressureValues = fabs((double)(i - j));
                EXPECT_EQ(RESULT_OK, pressure.assignReal(fieldcache, 1, &pressureValues));
            }
            if (hasTemperature)
            {
                double temperatureValues = j*j + i*i;
                EXPECT_EQ(RESULT_OK, temperature.assignReal(fieldcache, 1, &temperatureValues));
            }
        }

    Mesh mesh = fm.findMeshByDimension(2);
    EXPECT_TRUE(mesh.isValid());

    Elementbasis bilinearBasis = fm.createElementbasis(2, Elementbasis::FUNCTION_TYPE_LINEAR_LAGRANGE);
    EXPECT_TRUE(bilinearBasis.isValid());
    Elementfieldtemplate bilinearEft = mesh.createElementfieldtemplate(bilinearBasis);
    EXPECT_TRUE(bilinearEft.isValid());
    Elementbasis biquadraticBasis = fm.createElementbasis(2, Elementbasis::FUNCTION_TYPE_QUADRATIC_LAGRANGE);
    EXPECT_TRUE(biquadraticBasis.isValid());
    Elementfieldtemplate biquadraticEft = mesh.createElementfieldtemplate(biquadraticBasis);
    EXPECT_TRUE(biquadraticEft.isValid());

    int biquadraticLocalNodeIndexes[9] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    Elementtemplate elementtemplate_cpt = mesh.createElementtemplate();
    EXPECT_EQ(RESULT_OK, result = elementtemplate_cpt.setElementShapeType(Element::SHAPE_TYPE_SQUARE));
    EXPECT_EQ(RESULT_OK, result = elementtemplate_cpt.defineField(coordinates, /*componentNumber*/1, biquadraticEft));
    EXPECT_EQ(RESULT_OK, result = elementtemplate_cpt.defineField(coordinates, /*componentNumber*/2, bilinearEft));
    EXPECT_EQ(RESULT_OK, result = elementtemplate_cpt.defineField(pressure, /*componentNumber*/-1, bilinearEft));
    EXPECT_EQ(RESULT_OK, result = elementtemplate_cpt.defineField(temperature, /*componentNumber*/-1, biquadraticEft));
    Elementtemplate elementtemplate_cp = mesh.createElementtemplate();
    EXPECT_EQ(RESULT_OK, result = elementtemplate_cp.setElementShapeType(Element::SHAPE_TYPE_SQUARE));
    EXPECT_EQ(RESULT_OK, result = elementtemplate_cp.defineField(coordinates, /*componentNumber*/1, biquadraticEft));
    EXPECT_EQ(RESULT_OK, result = elementtemplate_cp.defineField(coordinates, /*componentNumber*/2, bilinearEft));
    EXPECT_EQ(RESULT_OK, result = elementtemplate_cp.defineField(pressure, /*componentNumber*/-1, bilinearEft));
    Elementtemplate elementtemplate_ct = mesh.createElementtemplate();
    EXPECT_EQ(RESULT_OK, result = elementtemplate_ct.setElementShapeType(Element::SHAPE_TYPE_SQUARE));
    EXPECT_EQ(RESULT_OK, result = elementtemplate_ct.defineField(coordinates, /*componentNumber*/1, biquadraticEft));
    EXPECT_EQ(RESULT_OK, result = elementtemplate_ct.defineField(coordinates, /*componentNumber*/2, bilinearEft));
    EXPECT_EQ(RESULT_OK, result = elementtemplate_ct.defineField(temperature, /*componentNumber*/-1, biquadraticEft));
    int nodeIdentifiers[9];
    for (int j = 0; j < 4; ++j)
    {
        for (int i = 0; i < 4; ++i)
        {
            const bool hasPressure = j < 3;
            const bool hasTemperature = j > 0;
            Elementtemplate elementtemplate;
            if (hasPressure)
            {
                if (hasTemperature)
                    elementtemplate = elementtemplate_cpt;
                else
                    elementtemplate = elementtemplate_cp;
            }
            else
                elementtemplate = elementtemplate_ct;
            const int elementIdentifier = j * 4 + i + 1;
            Element element = mesh.createElement(elementIdentifier, elementtemplate);

            const int baseNodeIdentifier = 18 * j + 2 * i + 1;
            for (int n = 0; n < 9; ++n)
            {
                nodeIdentifiers[n] = baseNodeIdentifier + (n / 3) * 9 + (n % 3);
            }
            const int bilinearNodeIdentifier[4] = { nodeIdentifiers[0], nodeIdentifiers[2], nodeIdentifiers[6], nodeIdentifiers[8] };
            EXPECT_EQ(RESULT_OK, element.setNodesByIdentifier(bilinearEft, 4, bilinearNodeIdentifier));
            EXPECT_EQ(RESULT_OK, element.setNodesByIdentifier(biquadraticEft, 9, nodeIdentifiers));
        }
    }
}

void check_mixed_template_squares(Fieldmodule& fm)
{
    int result;
    Field coordinates = fm.findFieldByName("coordinates");
    EXPECT_TRUE(coordinates.isValid());
    EXPECT_EQ(2, coordinates.getNumberOfComponents());
    EXPECT_TRUE(coordinates.isTypeCoordinate());
    Field pressure = fm.findFieldByName("pressure");
    EXPECT_TRUE(pressure.isValid());
    EXPECT_EQ(1, pressure.getNumberOfComponents());
    Field temperature = fm.findFieldByName("temperature");
    EXPECT_TRUE(temperature.isValid());
    EXPECT_EQ(1, temperature.getNumberOfComponents());

    EXPECT_EQ(OK, result = fm.defineAllFaces());
    Mesh mesh3d = fm.findMeshByDimension(3);
    EXPECT_EQ(0, mesh3d.getSize());
    Mesh mesh2d = fm.findMeshByDimension(2);
    int elementsCount = mesh2d.getSize();
    EXPECT_EQ(16, elementsCount);
    Mesh mesh1d = fm.findMeshByDimension(1);
    EXPECT_EQ(40, mesh1d.getSize());
    Nodeset nodes = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
    int nodesetSize = nodes.getSize();
    EXPECT_EQ(81, nodesetSize);
    Fieldcache fieldcache = fm.createFieldcache();
    EXPECT_TRUE(fieldcache.isValid());
    for (int e = 1; e <= elementsCount; ++e)
    {
        Element element = mesh2d.findElementByIdentifier(e);
        EXPECT_TRUE(element.isValid());
        Element::ShapeType shapeType = element.getShapeType();
        EXPECT_EQ(Element::SHAPE_TYPE_SQUARE, shapeType);
        EXPECT_EQ(OK, result = fieldcache.setElement(element));
        EXPECT_TRUE(coordinates.isDefinedAtLocation(fieldcache));
        bool hasPressure = pressure.isDefinedAtLocation(fieldcache);
        if (e < 13)
            EXPECT_TRUE(hasPressure);
        else
            EXPECT_FALSE(hasPressure);
        bool hasTemperature = temperature.isDefinedAtLocation(fieldcache);
        if (e > 4)
            EXPECT_TRUE(hasTemperature);
        else
            EXPECT_FALSE(hasTemperature);
    }
    for (int n = 1; n < nodesetSize; ++n)
    {
        Node node = nodes.findNodeByIdentifier(n);
        EXPECT_TRUE(node.isValid());
        EXPECT_EQ(OK, result = fieldcache.setNode(node));
        EXPECT_TRUE(coordinates.isDefinedAtLocation(fieldcache));
        const int i = (n - 1) % 9;
        const int j = (n - 1) / 9;
        const bool linearNode = (0 == (i % 2)) && (0 == (j % 2));
        const bool expectedHasPressure = (j < 7) && linearNode;
        const bool expectedHasTemperature = (j > 1);
        bool hasPressure = pressure.isDefinedAtLocation(fieldcache);
        if (expectedHasPressure)
            EXPECT_TRUE(hasPressure);
        else
            EXPECT_FALSE(hasPressure);
        bool hasTemperature = temperature.isDefinedAtLocation(fieldcache);
        if (expectedHasTemperature)
            EXPECT_TRUE(hasTemperature);
        else
            EXPECT_FALSE(hasTemperature);
    }

    FieldIsDefined pressureDefined = fm.createFieldIsDefined(pressure);
    EXPECT_TRUE(pressureDefined.isValid());
    MeshGroup pressureMesh = fm.createFieldGroup().createMeshGroup(mesh2d);
    EXPECT_EQ(OK, pressureMesh.addElementsConditional(pressureDefined));

    FieldIsDefined temperatureDefined = fm.createFieldIsDefined(temperature);
    EXPECT_TRUE(temperatureDefined.isValid());
    MeshGroup temperatureMesh = fm.createFieldGroup().createMeshGroup(mesh2d);
    EXPECT_EQ(OK, temperatureMesh.addElementsConditional(temperatureDefined));

    const int pointCount = 1;
    FieldMeshIntegral pressureIntegral = fm.createFieldMeshIntegral(pressure, coordinates, pressureMesh);
    EXPECT_TRUE(pressureIntegral.isValid());
    EXPECT_EQ(OK, result = pressureIntegral.setNumbersOfPoints(1, &pointCount));
    FieldMeshIntegral temperatureIntegral = fm.createFieldMeshIntegral(temperature, coordinates, temperatureMesh);
    EXPECT_TRUE(temperatureIntegral.isValid());
    EXPECT_EQ(OK, result = temperatureIntegral.setNumbersOfPoints(1, &pointCount));

    double pressureIntegralValue;
    EXPECT_EQ(OK, result = pressureIntegral.evaluateReal(fieldcache, 1, &pressureIntegralValue));
    EXPECT_DOUBLE_EQ(31.0, pressureIntegralValue);
    double temperatureIntegralValue;
    EXPECT_EQ(OK, result = temperatureIntegral.evaluateReal(fieldcache, 1, &temperatureIntegralValue));
    EXPECT_DOUBLE_EQ(584.0, temperatureIntegralValue);
}

}

// Test reading EX model, writing and re-reading in EX2 format
// 2D example with different templates for components of the coordinates field
// and for two different scalar fields including mix of bilinear and
// biquadratic elements, with latter two fields not defined on whole mesh.
TEST(ZincRegion, ex2_mixed_template_squares)
{
    ZincTestSetupCpp zinc;
    int result;

    create_mixed_template_squares(zinc.fm);
    check_mixed_template_squares(zinc.fm);

    std::string outFile = manageOutputFolderFieldML.getPath("/mixed_template_squares.ex2");
    // test writing and re-reading in EX2 format
    EXPECT_EQ(OK, result = zinc.root_region.writeFile(outFile.c_str()));
    Region testRegion1 = zinc.root_region.createChild("test1");
    EXPECT_EQ(OK, result = testRegion1.readFile(outFile.c_str()));
    Fieldmodule testFm1 = testRegion1.getFieldmodule();
    check_mixed_template_squares(testFm1);
}

// Test reading EX model, writing and re-reading in FieldML format
// 2D example with different templates for components of the coordinates field
// and for two different scalar fields including mix of bilinear and
// biquadratic elements, with latter two fields not defined on whole mesh.
TEST(ZincRegion, fieldml_mixed_template_squares)
{
    ZincTestSetupCpp zinc;
    int result;

    create_mixed_template_squares(zinc.fm);
    check_mixed_template_squares(zinc.fm);

    std::string outFile = manageOutputFolderFieldML.getPath("/mixed_template_squares.fieldml");
    // test writing and re-reading in FieldML format
    EXPECT_EQ(OK, result = zinc.root_region.writeFile(outFile.c_str()));
    Region testRegion2 = zinc.root_region.createChild("test2");
    EXPECT_EQ(OK, result = testRegion2.readFile(outFile.c_str()));
    Fieldmodule testFm2 = testRegion2.getFieldmodule();
    check_mixed_template_squares(testFm2);
}

namespace {

void check_lines_unit_scale_factors_model(Fieldmodule& fm)
{
    int result;
    Field coordinates = fm.findFieldByName("coordinates");
    EXPECT_TRUE(coordinates.isValid());
    EXPECT_EQ(2, coordinates.getNumberOfComponents());
    EXPECT_TRUE(coordinates.isTypeCoordinate());

    EXPECT_EQ(OK, result = fm.defineAllFaces());
    Mesh mesh1d = fm.findMeshByDimension(1);
    const int elementsCount = mesh1d.getSize();
    EXPECT_EQ(4, elementsCount);
    Nodeset nodes = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
    EXPECT_EQ(4, nodes.getSize());
    for (int e = 1; e <= elementsCount; ++e)
    {
        Element element = mesh1d.findElementByIdentifier(e);
        EXPECT_TRUE(element.isValid());
        Element::ShapeType shapeType = element.getShapeType();
        EXPECT_EQ(Element::SHAPE_TYPE_LINE, shapeType);
    }

    const double valueOne = 1.0;
    Field one = fm.createFieldConstant(1, &valueOne);
    FieldMeshIntegral length = fm.createFieldMeshIntegral(one, coordinates, mesh1d);
    EXPECT_TRUE(length.isValid());

    Fieldcache cache = fm.createFieldcache();
    double outLength;
    EXPECT_EQ(OK, result = length.evaluateReal(cache, 1, &outLength));
    ASSERT_DOUBLE_EQ(4.0, outLength);
}

}

// Many EX files multiply all element parameters by stored unit scale factors
// even for Lagrange/Simplex bases that do not need them, and these are
// removed when writing to FieldML.
// This example reads a 1-D model with a mix of elements interpolating with
// both stored unit scale factors, and the same basis with no scale factors.
// It tests matching the different cases to the same element field template
// and also overwriting the definition when re-reading from FieldML.
TEST(ZincRegion, lines_unit_scale_factors)
{
    ZincTestSetupCpp zinc;
    int result;

    EXPECT_EQ(OK, result = zinc.root_region.readFile(
        resourcePath("fieldio/lines_unit_scale_factors.exfile").c_str()));
    check_lines_unit_scale_factors_model(zinc.fm);

    std::string outFile = manageOutputFolderFieldML.getPath("/lines_unit_scale_factors.fieldml");
    // test writing and re-reading in FieldML format
    EXPECT_EQ(OK, result = zinc.root_region.writeFile(outFile.c_str()));
    // the following tests overwriting element fields using stored unit scale factors
    // by element fields in the FieldML file which have no scaling
    EXPECT_EQ(OK, result = zinc.root_region.readFile(outFile.c_str()));
    check_lines_unit_scale_factors_model(zinc.fm);
}

// Test alternating the local node ordering but maintaining consistent
// local nodes for the different ordering is output successfully
TEST(ZincRegion, lines_alternate_node_order)
{
    ZincTestSetupCpp zinc;
    int result;

    EXPECT_EQ(RESULT_OK, result = zinc.root_region.readFile(
        resourcePath("fieldio/lines_alternate_node_order.exfile").c_str()));
    check_lines_unit_scale_factors_model(zinc.fm);

    std::string outFile = manageOutputFolderFieldML.getPath("/lines_alternate_node_order.fieldml");
    // test writing and re-reading in FieldML format
    EXPECT_EQ(RESULT_OK, result = zinc.root_region.writeFile(outFile.c_str()));
    Region testRegion1 = zinc.root_region.createChild("test1");
    EXPECT_EQ(RESULT_OK, result = testRegion1.readFile(outFile.c_str()));
    Fieldmodule testFm1 = testRegion1.getFieldmodule();
    check_lines_unit_scale_factors_model(testFm1);
}

// Test writing models with inconsistent local-to-global-node map
// for the same basis in an element.
TEST(ZincRegion, lines_inconsistent_node_order)
{
    ZincTestSetupCpp zinc;
    int result;

    EXPECT_EQ(RESULT_OK, result = zinc.root_region.readFile(
        resourcePath("fieldio/lines_inconsistent_node_order.exfile").c_str()));
    check_lines_unit_scale_factors_model(zinc.fm);

    std::string outFile = manageOutputFolderFieldML.getPath("/lines_inconsistent_node_order.fieldml");
    // test writing and re-reading in FieldML format
    EXPECT_EQ(RESULT_OK, result = zinc.root_region.writeFile(outFile.c_str()));
    Region testRegion1 = zinc.root_region.createChild("test1");
    EXPECT_EQ(RESULT_OK, result = testRegion1.readFile(outFile.c_str()));
    Fieldmodule testFm1 = testRegion1.getFieldmodule();
    check_lines_unit_scale_factors_model(testFm1);
}

namespace {

void check_cube_element_xi_model(Fieldmodule& fm)
{
    int result;
    Field coordinates = fm.findFieldByName("coordinates");
    EXPECT_TRUE(coordinates.isValid());
    EXPECT_EQ(3, coordinates.getNumberOfComponents());
    EXPECT_TRUE(coordinates.isTypeCoordinate());

    const double offsetValues[3] = { 0.1, 0.2, -0.35 };
    FieldConstant offset = fm.createFieldConstant(3, offsetValues);
    EXPECT_TRUE(offset.isValid());
    FieldAdd bob = coordinates + offset;
    EXPECT_TRUE(bob.isValid());
    EXPECT_EQ(RESULT_OK, result = bob.setName("bob"));

    Field element_xi = fm.findFieldByName("element_xi");
    EXPECT_TRUE(element_xi.isValid());
    EXPECT_EQ(Field::VALUE_TYPE_MESH_LOCATION, element_xi.getValueType());
    Field hostbob = fm.createFieldEmbedded(bob, element_xi);
    EXPECT_TRUE(hostbob.isValid());

    Mesh mesh3d = fm.findMeshByDimension(3);
    EXPECT_EQ(1, mesh3d.getSize());
    Nodeset nodes = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
    EXPECT_EQ(8, nodes.getSize());
    Nodeset datapoints = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_DATAPOINTS);
    EXPECT_EQ(5, datapoints.getSize());

    Element element1 = mesh3d.findElementByIdentifier(1);
    EXPECT_TRUE(element1.isValid());

    const double xiExpected[5][3] = {
        { 0.25, 0.25, 0.75 },
        { 0.25, 0.50, 0.75 },
        { 1.00, 0.25, 0.75 },
        { 1.00, 1.00, 1.00 },
        { 0.00, 0.00, 0.00 }
    };
    double xiOut[3];
    double hostbobOut[3];
    const double tolerance = 1.0E-8;
    Fieldcache cache = fm.createFieldcache();
    for (int i = 0; i < 5; ++i)
    {
        Node datapoint = datapoints.findNodeByIdentifier(i + 1);
        EXPECT_TRUE(datapoint.isValid());
        EXPECT_EQ(RESULT_OK, result = cache.setNode(datapoint));
        Element elementOut = element_xi.evaluateMeshLocation(cache, 3, xiOut);
        EXPECT_EQ(element1, elementOut);
        EXPECT_NEAR(xiExpected[i][0], xiOut[0], tolerance);
        EXPECT_NEAR(xiExpected[i][1], xiOut[1], tolerance);
        EXPECT_NEAR(xiExpected[i][2], xiOut[2], tolerance);
        EXPECT_EQ(RESULT_OK, result = hostbob.evaluateReal(cache, 3, hostbobOut));
        EXPECT_NEAR(xiExpected[i][0] + offsetValues[0], hostbobOut[0], tolerance);
        EXPECT_NEAR(xiExpected[i][1] + offsetValues[1], hostbobOut[1], tolerance);
        EXPECT_NEAR(xiExpected[i][2] + offsetValues[2], hostbobOut[2], tolerance);
    }
}

}

TEST(FieldIO, cube_element_xi)
{
    ZincTestSetupCpp zinc;
    int result;

    // Test can't merge element:xi locations unless host elements have been defined first
    EXPECT_EQ(RESULT_ERROR_INCOMPATIBLE_DATA, result = zinc.root_region.readFile(
        resourcePath("fieldio/cube_element_xi_old.exdata").c_str()));

    // read the cube host mesh
    EXPECT_EQ(RESULT_OK, result = zinc.root_region.readFile(
        resourcePath("cube.ex2").c_str()));

    // Test can't read old EX format that had different dimension elements for element:xi field
    EXPECT_EQ(RESULT_ERROR_GENERAL, result = zinc.root_region.readFile(
        resourcePath("fieldio/cube_element_xi_old_fail.exdata").c_str()));

    // now read the datapoints with element:xi field
    EXPECT_EQ(RESULT_OK, result = zinc.root_region.readFile(
        resourcePath("fieldio/cube_element_xi_old.exdata").c_str()));
    check_cube_element_xi_model(zinc.fm);

    std::string outFile = manageOutputFolderFieldML.getPath("/cube_element_xi.ex2");
    // test writing datapoints and re-reading in EX2 format
    StreaminformationRegion sir = zinc.root_region.createStreaminformationRegion();
    EXPECT_TRUE(sir.isValid());
    StreamresourceFile srf = sir.createStreamresourceFile(outFile.c_str());
    EXPECT_TRUE(srf.isValid());
    EXPECT_EQ(RESULT_OK, result = sir.setResourceDomainTypes(srf, Field::DOMAIN_TYPE_DATAPOINTS));
    EXPECT_EQ(RESULT_OK, result = zinc.root_region.write(sir));

    Region testRegion1 = zinc.root_region.createChild("test1");
    EXPECT_EQ(RESULT_OK, result = testRegion1.readFile(resourcePath("cube.ex2").c_str()));
    EXPECT_EQ(RESULT_OK, result = testRegion1.readFile(outFile.c_str()));
    Fieldmodule testFm1 = testRegion1.getFieldmodule();
    check_cube_element_xi_model(testFm1);
}

namespace {

void check_ex_element_grid_constant_indexed_fields(Fieldmodule& fm)
{
    int result;

    Field coordinates = fm.findFieldByName("coordinates");
    EXPECT_TRUE(coordinates.isValid());
    EXPECT_EQ(3, coordinates.getNumberOfComponents());
    EXPECT_TRUE(coordinates.isTypeCoordinate());
    EXPECT_EQ(Field::VALUE_TYPE_REAL, coordinates.getValueType());
    Field materialType = fm.findFieldByName("material_type");
    EXPECT_TRUE(materialType.isValid());
    EXPECT_EQ(1, materialType.getNumberOfComponents());
    // integer type is not yet presented in the public API, currently reports as REAL
    EXPECT_EQ(Field::VALUE_TYPE_REAL, materialType.getValueType());
    Field materialName = fm.findFieldByName("material_name");
    EXPECT_TRUE(materialName.isValid());
    EXPECT_EQ(1, materialName.getNumberOfComponents());
    EXPECT_EQ(Field::VALUE_TYPE_STRING, materialName.getValueType());
    Field conductivity = fm.findFieldByName("conductivity");
    EXPECT_TRUE(conductivity.isValid());
    EXPECT_EQ(1, conductivity.getNumberOfComponents());
    EXPECT_EQ(Field::VALUE_TYPE_REAL, conductivity.getValueType());

    Field magneticFieldVector = fm.findFieldByName("magnetic field vector");
    EXPECT_TRUE(magneticFieldVector.isValid());
    EXPECT_EQ(3, magneticFieldVector.getNumberOfComponents());
    EXPECT_EQ(Field::VALUE_TYPE_REAL, magneticFieldVector.getValueType());

    Field potential = fm.findFieldByName("potential");
    EXPECT_TRUE(potential.isValid());
    EXPECT_EQ(1, potential.getNumberOfComponents());
    EXPECT_EQ(Field::VALUE_TYPE_REAL, potential.getValueType());

    Mesh mesh3d = fm.findMeshByDimension(3);
    EXPECT_TRUE(mesh3d.isValid());
    Fieldcache cache = fm.createFieldcache();
    EXPECT_TRUE(cache.isValid());

    const double xi[3] = { 0.1, 0.2, 0.4 };
    const double expectedConductivityOut[2] = { 27.4, 20.2 };
    const double expectedCoordinatesOut[2][3] = { { 1, 2, 4 }, { 11, 2, 4 } };
    const double expectedMagneticFieldVectorOut[3] = { 0.1, 0.2, 0.9 };
    const char *expectedMaterialNameOut[2] = { "copper", "\"mixed\" alloy" };
    const double expectedMaterialTypeOut[2] = { 1, 3 };
    const double expectedPotentialOut[2] = { 4.4947897920000015, 4.3176384679999993 };
    for (int e = 0; e < 2; ++e)
    {
        double conductivityOut, coordinatesOut[3], magneticFieldVectorOut[3], materialTypeOut, potentialOut;

        Element element = mesh3d.findElementByIdentifier(e + 1);
        EXPECT_TRUE(element.isValid());
        EXPECT_EQ(RESULT_OK, result = cache.setMeshLocation(element, 3, xi));

        EXPECT_EQ(RESULT_OK, result = conductivity.evaluateReal(cache, 1, &conductivityOut));
        EXPECT_DOUBLE_EQ(expectedConductivityOut[e], conductivityOut);

        EXPECT_EQ(RESULT_OK, result = coordinates.evaluateReal(cache, 3, coordinatesOut));
        for (int c = 0; c < 3; ++c)
            EXPECT_NEAR(expectedCoordinatesOut[e][c], coordinatesOut[c], 1.0E-6);

        EXPECT_EQ(RESULT_OK, result = magneticFieldVector.evaluateReal(cache, 3, magneticFieldVectorOut));
        // constant field should be same in both elements:
        for (int c = 0; c < 3; ++c)
            EXPECT_DOUBLE_EQ(expectedMagneticFieldVectorOut[c], magneticFieldVectorOut[c]);

        char *materialNameOut = materialName.evaluateString(cache);
        EXPECT_STREQ(expectedMaterialNameOut[e], materialNameOut);
        cmzn_deallocate(materialNameOut);
        materialNameOut = 0;

        EXPECT_EQ(RESULT_OK, result = materialType.evaluateReal(cache, 1, &materialTypeOut));
        EXPECT_DOUBLE_EQ(expectedMaterialTypeOut[e], materialTypeOut);

        EXPECT_EQ(RESULT_OK, result = potential.evaluateReal(cache, 1, &potentialOut));
        EXPECT_NEAR(expectedPotentialOut[e], potentialOut, 1.0E-6);
    }
}

}

// Tests grid-based, constant and indexed fields. Taken from Cmgui example a/exelem_formats
TEST(FieldIO, ex_element_grid_constant_indexed_fields)
{
    ZincTestSetupCpp zinc;
    int result;

    EXPECT_EQ(RESULT_OK, result = zinc.root_region.readFile(
        resourcePath("fieldio/block_grid.exfile").c_str()));
    check_ex_element_grid_constant_indexed_fields(zinc.fm);

    std::string outFile = manageOutputFolderFieldML.getPath("/block_grid.exf");
    // test writing and re-reading in latest EX format
    EXPECT_EQ(RESULT_OK, result = zinc.root_region.writeFile(outFile.c_str()));
    Region testRegion1 = zinc.root_region.createChild("test1");
    EXPECT_EQ(RESULT_OK, result = testRegion1.readFile(outFile.c_str()));
    Fieldmodule testFm1 = testRegion1.getFieldmodule();
    check_ex_element_grid_constant_indexed_fields(testFm1);
}

namespace {

void check_ex_special_node_fields(Fieldmodule& fm)
{
    Field cell_type = fm.findFieldByName("cell_type");
    EXPECT_TRUE(cell_type.isValid());
    EXPECT_EQ(1, cell_type.getNumberOfComponents());
    // integer type is not yet public, so returns REAL:
    EXPECT_EQ(Field::VALUE_TYPE_REAL, cell_type.getValueType());

    Field constant_real3 = fm.findFieldByName("constant_real3");
    EXPECT_TRUE(constant_real3.isValid());
    EXPECT_EQ(3, constant_real3.getNumberOfComponents());
    EXPECT_EQ(Field::VALUE_TYPE_REAL, constant_real3.getValueType());

    Field indexed_real = fm.findFieldByName("indexed_real");
    EXPECT_TRUE(indexed_real.isValid());
    EXPECT_EQ(1, indexed_real.getNumberOfComponents());
    EXPECT_EQ(Field::VALUE_TYPE_REAL, indexed_real.getValueType());

    Field indexed_int2 = fm.findFieldByName("indexed_int2");
    EXPECT_TRUE(indexed_int2.isValid());
    EXPECT_EQ(2, indexed_int2.getNumberOfComponents());
    // integer type is not yet public, so returns REAL:
    EXPECT_EQ(Field::VALUE_TYPE_REAL, indexed_int2.getValueType());

    Field constant_string = fm.findFieldByName("constant_string");
    EXPECT_TRUE(constant_string.isValid());
    EXPECT_EQ(1, constant_string.getNumberOfComponents());
    EXPECT_EQ(Field::VALUE_TYPE_STRING, constant_string.getValueType());

    Field indexed_string = fm.findFieldByName("indexed_string");
    EXPECT_TRUE(indexed_string.isValid());
    EXPECT_EQ(1, indexed_string.getNumberOfComponents());
    EXPECT_EQ(Field::VALUE_TYPE_STRING, indexed_string.getValueType());

    Field general_string = fm.findFieldByName("general_string");
    EXPECT_TRUE(general_string.isValid());
    EXPECT_EQ(1, general_string.getNumberOfComponents());
    EXPECT_EQ(Field::VALUE_TYPE_STRING, general_string.getValueType());

    Nodeset nodeset = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
    EXPECT_TRUE(nodeset.isValid());
    Fieldcache cache = fm.createFieldcache();
    EXPECT_TRUE(cache.isValid());

    const int expected_cell_type[8] = { 1, 1, 1, 1, 2, 3, 2, 3 };
    const double expected_constant_real3[3] = { 0.1, 0.2, 0.5 };
    const double expected_indexed_real[3] = { 10.5, 15.5, 25.5 };
    const double expected_indexed_int2[3][2] = { { 1, 7 }, { 2, 8 }, { 3, 9 } };
    const char expected_constant_string[] = "\"Constant\" string";
    const char *expected_indexed_string[3] = { "Cell type 1", "Cell type 2", "\"Type-3 Cell\"" };
    const char *expected_general_string[8] = { "one", "two", "three", "four", "five", "six", "\'seven\'", "eight" };

    for (int n = 0; n < 8; ++n)
    {
        Node node = nodeset.findNodeByIdentifier(n + 1);
        EXPECT_TRUE(node.isValid());
        EXPECT_EQ(RESULT_OK, cache.setNode(node));

        double cell_type_out_double = -1.0;
        // don't have evaluateInteger yet:
        EXPECT_EQ(RESULT_OK, cell_type.evaluateReal(cache, 1, &cell_type_out_double));
        EXPECT_DOUBLE_EQ(static_cast<double>(expected_cell_type[n]), cell_type_out_double);
        const int cell_type_out = static_cast<int>(cell_type_out_double + 0.5);
        EXPECT_EQ(expected_cell_type[n], cell_type_out);

        double constant_real3_out[3];
        EXPECT_EQ(RESULT_OK, constant_real3.evaluateReal(cache, 3, constant_real3_out));
        for (int c = 0; c < 3; ++c)
        {
            EXPECT_DOUBLE_EQ(expected_constant_real3[c], constant_real3_out[c]);
        }

        const int cell_type_index = expected_cell_type[n] - 1;

        double indexed_real_out;
        EXPECT_EQ(RESULT_OK, indexed_real.evaluateReal(cache, 1, &indexed_real_out));
        EXPECT_DOUBLE_EQ(expected_indexed_real[cell_type_index], indexed_real_out);

        double indexed_int2_out_double[3];
        EXPECT_EQ(RESULT_OK, indexed_int2.evaluateReal(cache, 3, indexed_int2_out_double));
        for (int c = 0; c < 2; ++c)
        {
            EXPECT_DOUBLE_EQ(static_cast<double>(expected_indexed_int2[cell_type_index][c]), indexed_int2_out_double[c]);
        }

        char *constant_string_out = constant_string.evaluateString(cache);
        EXPECT_STREQ(expected_constant_string, constant_string_out);
        cmzn_deallocate(constant_string_out);

        char *indexed_string_out = indexed_string.evaluateString(cache);
        EXPECT_STREQ(expected_indexed_string[cell_type_index], indexed_string_out);
        cmzn_deallocate(indexed_string_out);

        char *general_string_out = general_string.evaluateString(cache);
        EXPECT_STREQ(expected_general_string[n], general_string_out);
        cmzn_deallocate(general_string_out);
    }
}

}

// Tests grid-based, constant and indexed fields. Taken from Cmgui example a/exnode_formats
TEST(FieldIO, ex_special_node_fields)
{
    ZincTestSetupCpp zinc;
    int result;

    EXPECT_EQ(RESULT_OK, result = zinc.root_region.readFile(
        resourcePath("fieldio/special_node_fields.exnode").c_str()));
    check_ex_special_node_fields(zinc.fm);

    std::string outFile = manageOutputFolderFieldML.getPath("/special_node_fields.ex2");
    // test writing and re-reading in EX2 format
    EXPECT_EQ(RESULT_OK, result = zinc.root_region.writeFile(outFile.c_str()));
    Region testRegion1 = zinc.root_region.createChild("test1");
    EXPECT_EQ(RESULT_OK, result = testRegion1.readFile(outFile.c_str()));
    Fieldmodule testFm1 = testRegion1.getFieldmodule();
    check_ex_special_node_fields(testFm1);
}

namespace {

void createVariableNodeVersionsWithTime2d(Fieldmodule& fm)
{
    FieldFiniteElement coordinates = fm.createFieldFiniteElement(/*numberOfComponents*/3);
    EXPECT_TRUE(coordinates.isValid());
    EXPECT_EQ(RESULT_OK, coordinates.setName("coordinates"));
    EXPECT_EQ(RESULT_OK, coordinates.setTypeCoordinate(true));
    EXPECT_EQ(RESULT_OK, coordinates.setCoordinateSystemType(Field::COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN));
    EXPECT_EQ(RESULT_OK, coordinates.setManaged(true));
    EXPECT_EQ(RESULT_OK, coordinates.setComponentName(1, "x"));
    EXPECT_EQ(RESULT_OK, coordinates.setComponentName(2, "y"));
    EXPECT_EQ(RESULT_OK, coordinates.setComponentName(3, "z"));

    Nodeset nodeset = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
    EXPECT_TRUE(nodeset.isValid());

    const double times[3] = { 0.0, 1.0, 2.0 };
    Timesequence timesequence = fm.getMatchingTimesequence(3, times);
    EXPECT_TRUE(timesequence.isValid());

    // corner nodes have all 3 components defined
    Nodetemplate nodetemplate = nodeset.createNodetemplate();
    EXPECT_TRUE(nodetemplate.isValid());
    EXPECT_EQ(RESULT_OK, nodetemplate.defineField(coordinates));
    EXPECT_EQ(RESULT_OK, nodetemplate.setTimesequence(coordinates, timesequence));
    Timesequence tmpTimesequence = nodetemplate.getTimesequence(coordinates);
    EXPECT_EQ(timesequence, tmpTimesequence);

    // midside and centre nodes only have z defined:
    Nodetemplate nodetemplatez = nodeset.createNodetemplate();
    EXPECT_TRUE(nodetemplatez.isValid());
    EXPECT_EQ(RESULT_OK, nodetemplatez.defineField(coordinates));
    EXPECT_EQ(RESULT_OK, nodetemplatez.setValueNumberOfVersions(coordinates, /*componentNumber*/1, Node::VALUE_LABEL_VALUE, 0));
    EXPECT_EQ(RESULT_OK, nodetemplatez.setValueNumberOfVersions(coordinates, /*componentNumber*/2, Node::VALUE_LABEL_VALUE, 0));
    EXPECT_EQ(RESULT_OK, nodetemplatez.setTimesequence(coordinates, timesequence));
    EXPECT_EQ(0, nodetemplatez.getValueNumberOfVersions(coordinates, /*componentNumber*/1, Node::VALUE_LABEL_VALUE));
    EXPECT_EQ(0, nodetemplatez.getValueNumberOfVersions(coordinates, /*componentNumber*/2, Node::VALUE_LABEL_VALUE));
    EXPECT_EQ(1, nodetemplatez.getValueNumberOfVersions(coordinates, /*componentNumber*/3, Node::VALUE_LABEL_VALUE));
    tmpTimesequence = nodetemplatez.getTimesequence(coordinates);
    EXPECT_EQ(timesequence, tmpTimesequence);

    Elementbasis bilinearBasis = fm.createElementbasis(2, Elementbasis::FUNCTION_TYPE_LINEAR_LAGRANGE);
    EXPECT_TRUE(bilinearBasis.isValid());
    Elementbasis biquadraticBasis = fm.createElementbasis(2, Elementbasis::FUNCTION_TYPE_QUADRATIC_LAGRANGE);
    EXPECT_TRUE(biquadraticBasis.isValid());

    Mesh mesh = fm.findMeshByDimension(2);
    EXPECT_TRUE(mesh.isValid());

    // x and y will use bilinear basis:
    Elementfieldtemplate bilinearEft = mesh.createElementfieldtemplate(bilinearBasis);
    EXPECT_TRUE(bilinearEft.isValid());
    // z will use biquadratic basis:
    Elementfieldtemplate biquadraticEft = mesh.createElementfieldtemplate(biquadraticBasis);
    EXPECT_TRUE(biquadraticEft.isValid());

    Elementtemplate elementtemplate = mesh.createElementtemplate();
    EXPECT_TRUE(elementtemplate.isValid());
    EXPECT_EQ(RESULT_OK, elementtemplate.setElementShapeType(Element::SHAPE_TYPE_SQUARE));
    EXPECT_EQ(RESULT_OK, elementtemplate.defineField(coordinates, /*componentNumber*/1, bilinearEft));
    EXPECT_EQ(RESULT_OK, elementtemplate.defineField(coordinates, /*componentNumber*/2, bilinearEft));
    EXPECT_EQ(RESULT_OK, elementtemplate.defineField(coordinates, /*componentNumber*/3, biquadraticEft));

    // create model
    const int elementCount1 = 2;
    const int elementCount2 = 1;
    const int nodeCount1 = elementCount1*2 + 1;
    const int nodeCount2 = elementCount2*2 + 1;
    const double nodeCount1Radians = PI / static_cast<double>(nodeCount1 - 2);

    fm.beginChange();
    Fieldcache cache = fm.createFieldcache();
    EXPECT_TRUE(cache.isValid());

    // create nodes
    int nodeIdentifier = 1;
    for (int n2 = 0; n2 < nodeCount2; ++n2)
    {
        for (int n1 = 0; n1 < nodeCount1; ++n1)
        {
            bool zOnly = ((n1 % 2) != 0) || ((n2 % 2) != 0);
            Node node = nodeset.createNode(nodeIdentifier++, zOnly ? nodetemplatez : nodetemplate);
            EXPECT_TRUE(node.isValid());
            EXPECT_EQ(RESULT_OK, cache.setNode(node));
            for (int ti = 0; ti < 2; ++ti)
            {
                const double time = static_cast<double>(ti);
                cache.setTime(time);
                const double c[3] =
                {
                    n1*(5.0 + time),
                    n2*(5.0) + 0.5*n1*n1*(n2 - 1)*time,
                    sin(nodeCount1Radians*n1)*time*4.0*cos(n2 - 1)
                };
                const int result = coordinates.setNodeParameters(cache, -1, Node::VALUE_LABEL_VALUE, /*version*/1, 3, c);
                if (zOnly)
                {
                    EXPECT_EQ(RESULT_WARNING_PART_DONE, result);
                    // test setting individual parameters
                    EXPECT_EQ(RESULT_ERROR_NOT_FOUND, coordinates.setNodeParameters(cache, 1, Node::VALUE_LABEL_VALUE, /*version*/1, 1, c));
                    EXPECT_EQ(RESULT_ERROR_NOT_FOUND, coordinates.setNodeParameters(cache, 2, Node::VALUE_LABEL_VALUE, /*version*/1, 1, c + 1));
                    EXPECT_EQ(RESULT_OK, coordinates.setNodeParameters(cache, 3, Node::VALUE_LABEL_VALUE, /*version*/1, 1, c + 2));
                }
                else
                {
                    EXPECT_EQ(RESULT_OK, result);
                }
            }
        }
    }

    // create elements
    int elementIdentifier = 1;
    for (int e2 = 0; e2 < elementCount2; ++e2)
    {
        for (int e1 = 0; e1 < elementCount1; ++e1)
        {
            Element element = mesh.createElement(elementIdentifier++, elementtemplate);
            EXPECT_TRUE(element.isValid());
            const int ni = e2*nodeCount1 + e1*2 + 1;
            const int biquadraticNodeIdentifiers[9] =
            {
                ni, ni + 1, ni + 2,
                ni + nodeCount1, ni + nodeCount1 + 1, ni + nodeCount1 + 2,
                ni + 2*nodeCount1, ni + 2*nodeCount1 + 1, ni + 2*nodeCount1 + 2,
            };
            const int bilinearNodeIdentifiers[4] =
            {
                ni, ni + 2, ni + 2*nodeCount1, ni + 2*nodeCount1 + 2
            };
            EXPECT_EQ(RESULT_OK, element.setNodesByIdentifier(biquadraticEft, 9, biquadraticNodeIdentifiers));
            EXPECT_EQ(RESULT_OK, element.setNodesByIdentifier(bilinearEft, 4, bilinearNodeIdentifiers));
        }
    }

    fm.endChange();
}

void checkVariableNodeVersionsWithTime2d(Fieldmodule& fm)
{
    FieldFiniteElement coordinates = fm.findFieldByName("coordinates").castFiniteElement();
    EXPECT_TRUE(coordinates.isValid());
    EXPECT_EQ(3, coordinates.getNumberOfComponents());
    EXPECT_TRUE(coordinates.isTypeCoordinate());

    const int elementCount1 = 2;
    const int elementCount2 = 1;
    const int nodeCount1 = elementCount1*2 + 1;
    const int nodeCount2 = elementCount2*2 + 1;
    const double tolerance = 1.0E-6;

    EXPECT_EQ(RESULT_OK, fm.defineAllFaces());
    Mesh mesh2d = fm.findMeshByDimension(2);
    EXPECT_EQ(elementCount1*elementCount2, mesh2d.getSize());
    Mesh mesh1d = fm.findMeshByDimension(1);
    EXPECT_EQ((elementCount1 + 1)*elementCount2 + elementCount1*(elementCount2 + 1), mesh1d.getSize());
    Nodeset nodeset = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
    EXPECT_EQ(nodeCount1*nodeCount2, nodeset.getSize());

    FieldNodeValue nodeValue = fm.createFieldNodeValue(coordinates, Node::VALUE_LABEL_VALUE, /*version*/1);
    EXPECT_TRUE(nodeValue.isValid());
    const double valueOne = 1.0;
    Field one = fm.createFieldConstant(1, &valueOne);
    FieldMeshIntegral area = fm.createFieldMeshIntegral(one, coordinates, mesh2d);
    EXPECT_TRUE(area.isValid());
    const int numGaussPoints = 4;
    EXPECT_EQ(RESULT_OK, area.setNumbersOfPoints(1, &numGaussPoints));

    Fieldcache cache = fm.createFieldcache();
    EXPECT_TRUE(cache.isValid());

    Node node15 = nodeset.findNodeByIdentifier(15);
    EXPECT_TRUE(node15.isValid());
    EXPECT_EQ(RESULT_OK, cache.setNode(node15));
    EXPECT_TRUE(coordinates.isDefinedAtLocation(cache));
    EXPECT_TRUE(coordinates.hasParametersAtLocation(cache));
    EXPECT_TRUE(nodeValue.isDefinedAtLocation(cache));
    const double expected_coordinates_node15[3][3] =
    {
        { 20.0, 10.0, 0.0 },
        { 22.0, 14.0, -0.93583104521023752 },
        { 24.0, 18.0, -1.8716620904204750 }
    };
    const double expected_area[3] =
    {
        200.0,
        296.07757780219970,
        427.23042426514678
    };
    double areaOut, coordinatesOut[3];
    for (int ti = 0; ti < 3; ++ti)
    {
        const double time = 0.5*ti;
        EXPECT_EQ(RESULT_OK, cache.setTime(time));
        EXPECT_EQ(RESULT_OK, coordinates.evaluateReal(cache, 3, coordinatesOut));
        for (int c = 0; c < 3; ++c)
        {
            EXPECT_NEAR(expected_coordinates_node15[ti][c], coordinatesOut[c], tolerance);
        }
        EXPECT_EQ(RESULT_OK, nodeValue.evaluateReal(cache, 3, coordinatesOut));
        for (int c = 0; c < 3; ++c)
        {
            EXPECT_NEAR(expected_coordinates_node15[ti][c], coordinatesOut[c], tolerance);
        }
        EXPECT_EQ(RESULT_OK, coordinates.getNodeParameters(cache, /*components=all*/-1, Node::VALUE_LABEL_VALUE, /*version*/1, 3, coordinatesOut));
        for (int c = 0; c < 3; ++c)
        {
            EXPECT_NEAR(expected_coordinates_node15[ti][c], coordinatesOut[c], tolerance);
        }
        EXPECT_EQ(RESULT_OK, area.evaluateReal(cache, 1, &areaOut));
        EXPECT_NEAR(expected_area[ti], areaOut, tolerance);
    }

    Node node12 = nodeset.findNodeByIdentifier(12);
    EXPECT_TRUE(node12.isValid());
    EXPECT_EQ(RESULT_OK, cache.setNode(node12));
    // coordinates has parameters at node 12, but is not defined and
    // can't be evaluated since not all components exist
    EXPECT_FALSE(coordinates.isDefinedAtLocation(cache));
    EXPECT_TRUE(coordinates.hasParametersAtLocation(cache));
    // nodeValue silently ignores missing components
    EXPECT_TRUE(nodeValue.isDefinedAtLocation(cache));
    const double expected_coordinates_node12[5][3] =
    {
        { 0.0, 0.0, 0.0 },
        { 0.0, 0.0, 0.93583104521023774 },
        { 0.0, 0.0, 1.871662090420475 }
    };
    for (int ti = 0; ti < 3; ++ti)
    {
        const double time = 0.5*ti;
        EXPECT_EQ(RESULT_OK, cache.setTime(time));
        EXPECT_EQ(RESULT_ERROR_GENERAL, coordinates.evaluateReal(cache, 3, coordinatesOut));
        EXPECT_EQ(RESULT_OK, nodeValue.evaluateReal(cache, 3, coordinatesOut));
        for (int c = 0; c < 3; ++c)
        {
            EXPECT_NEAR(expected_coordinates_node12[ti][c], coordinatesOut[c], tolerance);
        }
        EXPECT_EQ(RESULT_WARNING_PART_DONE, coordinates.getNodeParameters(cache, /*components=all*/-1, Node::VALUE_LABEL_VALUE, /*version*/1, 3, coordinatesOut));
        for (int c = 0; c < 3; ++c)
        {
            EXPECT_NEAR(expected_coordinates_node12[ti][c], coordinatesOut[c], tolerance);
        }
    }
}

}

/** @return  Result of region write operation */
int writeNodesAtTime(Region& region, double time, const char *filename)
{
    StreaminformationRegion sir = region.createStreaminformationRegion();
    StreamresourceFile srf = sir.createStreamresourceFile(filename);
    sir.setResourceAttributeReal(srf, StreaminformationRegion::ATTRIBUTE_TIME, time);
    sir.setResourceDomainTypes(srf, Field::DOMAIN_TYPE_NODES);
    return region.write(sir);
}

// Test a model mixing 2-D bilinear and biquadratic bases for different
// components of coordinate field, testing evaluation of midside nodes which
// don't have all components.
TEST(FieldIO, variableNodeVersionsWithTime2d)
{
    ZincTestSetupCpp zinc;
    int result;

    createVariableNodeVersionsWithTime2d(zinc.fm);
    checkVariableNodeVersionsWithTime2d(zinc.fm);

    // test writing and re-reading in EX2 format
    std::string nodes_time0 = std::string(TESTS_OUTPUT_LOCATION) + "/variable_node_versions_nodes_time0.ex2";
    std::string nodes_time1 = std::string(TESTS_OUTPUT_LOCATION) + "/variable_node_versions_nodes_time1.ex2";
    std::string nodes_elements = std::string(TESTS_OUTPUT_LOCATION) + "/variable_node_versions_elements.ex2";

    EXPECT_EQ(RESULT_OK, result = writeNodesAtTime(zinc.root_region, 0.0, nodes_time0.c_str()));
    EXPECT_EQ(RESULT_OK, result = writeNodesAtTime(zinc.root_region, 1.0, nodes_time1.c_str()));
    StreaminformationRegion sir = zinc.root_region.createStreaminformationRegion();
    StreamresourceFile srf = sir.createStreamresourceFile(nodes_elements.c_str());
    sir.setResourceDomainTypes(srf, Field::DOMAIN_TYPE_MESH1D | Field::DOMAIN_TYPE_MESH2D);
    EXPECT_EQ(RESULT_OK, result = zinc.root_region.write(sir));

    Region region2 = zinc.context.createRegion();
    sir = region2.createStreaminformationRegion();
    StreamresourceFile srf_nodes_time0 = sir.createStreamresourceFile(nodes_time0.c_str());
    // time is now stored in EX3 file, no need to specify on read
    //sir.setResourceAttributeReal(srf_nodes_time0, StreaminformationRegion::ATTRIBUTE_TIME, 0.0);
    StreamresourceFile srf_nodes_time1 = sir.createStreamresourceFile(nodes_time1.c_str());
    //sir.setResourceAttributeReal(srf_nodes_time1, StreaminformationRegion::ATTRIBUTE_TIME, 1.0);
    StreamresourceFile srf_elements = sir.createStreamresourceFile(nodes_elements.c_str());
    EXPECT_EQ(RESULT_OK, result = region2.read(sir));
    Fieldmodule fm2 = region2.getFieldmodule();
    checkVariableNodeVersionsWithTime2d(fm2);
}

namespace {

void createPartElementsModel(Fieldmodule& fm)
{
    FieldFiniteElement coordinates = fm.createFieldFiniteElement(/*numberOfComponents*/3);
    EXPECT_TRUE(coordinates.isValid());
    EXPECT_EQ(RESULT_OK, coordinates.setName("coordinates"));
    EXPECT_EQ(RESULT_OK, coordinates.setTypeCoordinate(true));
    EXPECT_EQ(RESULT_OK, coordinates.setCoordinateSystemType(Field::COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN));
    EXPECT_EQ(RESULT_OK, coordinates.setManaged(true));
    EXPECT_EQ(RESULT_OK, coordinates.setComponentName(1, "x"));
    EXPECT_EQ(RESULT_OK, coordinates.setComponentName(2, "y"));
    EXPECT_EQ(RESULT_OK, coordinates.setComponentName(3, "z"));

    Nodeset nodes = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
    EXPECT_TRUE(nodes.isValid());
    Nodetemplate nodetemplate = nodes.createNodetemplate();
    EXPECT_TRUE(nodetemplate.isValid());
    EXPECT_EQ(RESULT_OK, nodetemplate.defineField(coordinates));

    Mesh mesh3d = fm.findMeshByDimension(3);
    Elementbasis trilinearBasis = fm.createElementbasis(3, Elementbasis::FUNCTION_TYPE_LINEAR_LAGRANGE);
    EXPECT_TRUE(trilinearBasis.isValid());
    Elementfieldtemplate eft = mesh3d.createElementfieldtemplate(trilinearBasis);
    EXPECT_EQ(RESULT_OK, eft.setNumberOfLocalScaleFactors(8));
    EXPECT_TRUE(eft.isValid());
    for (int f = 1; f <= 8; ++f)
    {
        EXPECT_EQ(RESULT_OK, eft.setScaleFactorType(f, Elementfieldtemplate::SCALE_FACTOR_TYPE_NODE_PATCH));
        EXPECT_EQ(RESULT_OK, eft.setScaleFactorIdentifier(f, 1));
        EXPECT_EQ(RESULT_OK, eft.setTermScaling(f, 1, 1, &f));
    }

    Elementtemplate elementtemplate = mesh3d.createElementtemplate();
    EXPECT_TRUE(elementtemplate.isValid());
    EXPECT_EQ(RESULT_OK, elementtemplate.setElementShapeType(Element::SHAPE_TYPE_CUBE));
    EXPECT_EQ(RESULT_OK, elementtemplate.defineField(coordinates, -1, eft));

    Fieldcache cache = fm.createFieldcache();
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
    const int nodeIdentifiersError[8] = { 1, 2, 3, 4, 5, 6, 17, 8 };
    const double scaleFactors[8] = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };

    Element element1 = mesh3d.createElement(1, elementtemplate);
    // following fails because node 17 doesn't exist, leaving element without nodes
    EXPECT_EQ(RESULT_ERROR_ARGUMENT, element1.setNodesByIdentifier(eft, 8, nodeIdentifiersError));
    // setScaleFactors fails because nodes have not been set with node-based scale factors
    EXPECT_EQ(RESULT_ERROR_NOT_FOUND, element1.setScaleFactors(eft, 8, scaleFactors));

    Element element2 = mesh3d.createElement(2, elementtemplate);
    EXPECT_EQ(RESULT_OK, element2.setNodesByIdentifier(eft, 8, nodeIdentifiers));
    EXPECT_EQ(RESULT_OK, element2.setScaleFactors(eft, 8, scaleFactors));

    EXPECT_EQ(RESULT_WARNING_PART_DONE, fm.defineAllFaces());
}

void checkPartElementsModel(Fieldmodule& fm)
{
    FieldFiniteElement coordinates = fm.findFieldByName("coordinates").castFiniteElement();
    EXPECT_TRUE(coordinates.isValid());
    EXPECT_EQ(3, coordinates.getNumberOfComponents());
    EXPECT_TRUE(coordinates.isTypeCoordinate());

    Mesh mesh3d = fm.findMeshByDimension(3);
    EXPECT_EQ(2, mesh3d.getSize());
    Mesh mesh2d = fm.findMeshByDimension(2);
    EXPECT_EQ(6, mesh2d.getSize());
    Mesh mesh1d = fm.findMeshByDimension(1);
    EXPECT_EQ(12, mesh1d.getSize());
    Nodeset nodes = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
    EXPECT_EQ(8, nodes.getSize());
}

}

// test can write and read elements with local nodes and scale factors not yet set
TEST(FieldIO, partElements)
{
    ZincTestSetupCpp zinc;

    createPartElementsModel(zinc.fm);
    checkPartElementsModel(zinc.fm);

    std::string outFile = manageOutputFolderFieldML.getPath("/part_elements.ex2");
    EXPECT_EQ(RESULT_OK, zinc.root_region.writeFile(outFile.c_str()));

    Region testRegion1 = zinc.root_region.createChild("test1");
    EXPECT_EQ(RESULT_OK, testRegion1.readFile(outFile.c_str()));
    Fieldmodule testFm1 = testRegion1.getFieldmodule();
    checkPartElementsModel(testFm1);
}

namespace {

void createUnusedLocalNodesModel(Fieldmodule& fm)
{
    FieldFiniteElement coordinates = fm.createFieldFiniteElement(/*numberOfComponents*/2);
    EXPECT_TRUE(coordinates.isValid());
    EXPECT_EQ(RESULT_OK, coordinates.setName("coordinates"));
    EXPECT_EQ(RESULT_OK, coordinates.setTypeCoordinate(true));
    EXPECT_EQ(RESULT_OK, coordinates.setCoordinateSystemType(Field::COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN));
    EXPECT_EQ(RESULT_OK, coordinates.setManaged(true));
    EXPECT_EQ(RESULT_OK, coordinates.setComponentName(1, "x"));
    EXPECT_EQ(RESULT_OK, coordinates.setComponentName(2, "y"));

    Nodeset nodes = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
    EXPECT_TRUE(nodes.isValid());
    Nodetemplate nodetemplate = nodes.createNodetemplate();
    EXPECT_TRUE(nodetemplate.isValid());
    EXPECT_EQ(RESULT_OK, nodetemplate.defineField(coordinates));

    Mesh mesh2d = fm.findMeshByDimension(2);
    Elementbasis bilinearBasis = fm.createElementbasis(2, Elementbasis::FUNCTION_TYPE_LINEAR_LAGRANGE);
    EXPECT_TRUE(bilinearBasis.isValid());
    Elementfieldtemplate eft = mesh2d.createElementfieldtemplate(bilinearBasis);
    EXPECT_TRUE(eft.isValid());
    // note: 1 more local node and scale factor than necessary
    EXPECT_EQ(RESULT_OK, eft.setNumberOfLocalNodes(5));
    EXPECT_EQ(RESULT_OK, eft.setNumberOfLocalScaleFactors(4));
    EXPECT_EQ(RESULT_OK, eft.setTermNodeParameter(1, 1, 1, Node::VALUE_LABEL_VALUE, 1));
    EXPECT_EQ(RESULT_OK, eft.setTermNodeParameter(2, 1, 2, Node::VALUE_LABEL_VALUE, 1));
    EXPECT_EQ(RESULT_OK, eft.setTermNodeParameter(3, 1, 4, Node::VALUE_LABEL_VALUE, 1));
    EXPECT_EQ(RESULT_OK, eft.setTermNodeParameter(4, 1, 5, Node::VALUE_LABEL_VALUE, 1));

    EXPECT_TRUE(eft.isValid());
    EXPECT_FALSE(eft.validate());
    for (int f = 1; f <= 4; ++f)
        EXPECT_EQ(RESULT_OK, eft.setTermScaling(f, 1, 1, &f));
    EXPECT_TRUE(eft.validate());

    Elementtemplate elementtemplate = mesh2d.createElementtemplate();
    EXPECT_TRUE(elementtemplate.isValid());
    EXPECT_EQ(RESULT_OK, elementtemplate.setElementShapeType(Element::SHAPE_TYPE_SQUARE));
    EXPECT_EQ(RESULT_OK, elementtemplate.defineField(coordinates, -1, eft));

    Fieldcache cache = fm.createFieldcache();
    EXPECT_TRUE(cache.isValid());

    int nodeIdentifier = 1;
    for (int i = 0; i < 4; ++i)
    {
        const double x[2] = { static_cast<double>(i%2), static_cast<double>(i/2) };
        Node node = nodes.createNode(nodeIdentifier++, nodetemplate);
        EXPECT_EQ(RESULT_OK, cache.setNode(node));
        EXPECT_TRUE(node.isValid());
        EXPECT_EQ(RESULT_OK, coordinates.setNodeParameters(cache, -1, Node::VALUE_LABEL_VALUE, 1, 2, x));
    }

    // note 3rd node is invalid and not used
    const int nodeIdentifiers[5] = { 1, 2, -1, 3, 4 };
    const double scaleFactors[4] = { 1.0, 1.0, 1.0, 1.0 };

    Element element = mesh2d.createElement(1, elementtemplate);
    EXPECT_EQ(RESULT_OK, element.setNodesByIdentifier(eft, 5, nodeIdentifiers));
    EXPECT_EQ(RESULT_OK, element.setScaleFactors(eft, 4, scaleFactors));

    EXPECT_EQ(RESULT_OK, fm.defineAllFaces());
}

void checkUnusedLocalNodesModel(Fieldmodule& fm, bool first)
{
    FieldFiniteElement coordinates = fm.findFieldByName("coordinates").castFiniteElement();
    EXPECT_TRUE(coordinates.isValid());
    EXPECT_EQ(2, coordinates.getNumberOfComponents());
    EXPECT_TRUE(coordinates.isTypeCoordinate());

    Mesh mesh2d = fm.findMeshByDimension(2);
    EXPECT_EQ(1, mesh2d.getSize());
    Mesh mesh1d = fm.findMeshByDimension(1);
    EXPECT_EQ(4, mesh1d.getSize());
    Nodeset nodes = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
    EXPECT_EQ(4, nodes.getSize());

    Element element = mesh2d.findElementByIdentifier(1);
    EXPECT_TRUE(element.isValid());
    Elementfieldtemplate eft = element.getElementfieldtemplate(coordinates, -1);
    EXPECT_TRUE(eft.isValid());
    EXPECT_EQ(first ? 5 : 4, eft.getNumberOfLocalNodes());

    EXPECT_EQ(1, eft.getTermLocalNodeIndex(1, 1));
    EXPECT_EQ(2, eft.getTermLocalNodeIndex(2, 1));
    EXPECT_EQ(first ? 4 : 3, eft.getTermLocalNodeIndex(3, 1));
    EXPECT_EQ(first ? 5 : 4, eft.getTermLocalNodeIndex(4, 1));

    int sfi;
    for (int f = 1; f <= 4; ++f)
    {
        EXPECT_EQ(RESULT_OK, eft.getTermScaling(f, 1, 1, &sfi));
        EXPECT_EQ(f, sfi);
    }
}

}

// test can write and read a model not using all local nodes in EFT.
// Note when re-reading from EX format unused local nodes are lost
TEST(FieldIO, unusedLocalNodes)
{
    ZincTestSetupCpp zinc;

    createUnusedLocalNodesModel(zinc.fm);
    checkUnusedLocalNodesModel(zinc.fm, true);

    std::string outFile = manageOutputFolderFieldML.getPath("/unused_local_nodes.ex2");
    EXPECT_EQ(RESULT_OK, zinc.root_region.writeFile(outFile.c_str()));

    Region testRegion1 = zinc.root_region.createChild("test1");
    EXPECT_EQ(RESULT_OK, testRegion1.readFile(outFile.c_str()));
    Fieldmodule testFm1 = testRegion1.getFieldmodule();
    checkUnusedLocalNodesModel(testFm1, false);
}

/* test fix of bug where elements were read into same stream information
   before time-varying nodes; node merge crashed due to invalid optimisation
   tranferring time array from template node to actual node */
TEST(FieldIO, read_elements_before_time_varying_nodes)
{
    ZincTestSetupCpp zinc;

    StreaminformationRegion sir = zinc.root_region.createStreaminformationRegion();
    sir.createStreamresourceFile(resourcePath("fieldio/cube_element.ex2").c_str());
    StreamresourceFile fr1 = sir.createStreamresourceFile(resourcePath("fieldio/cube_node1.ex2").c_str());
    sir.setResourceAttributeReal(fr1, StreaminformationRegion::ATTRIBUTE_TIME, 1.0);
    StreamresourceFile fr3 = sir.createStreamresourceFile(resourcePath("fieldio/cube_node3.ex2").c_str());
    sir.setResourceAttributeReal(fr3, StreaminformationRegion::ATTRIBUTE_TIME, 3.0);
    // deliberately merge out-of-order
    StreamresourceFile fr2 = sir.createStreamresourceFile(resourcePath("fieldio/cube_node2.ex2").c_str());
    sir.setResourceAttributeReal(fr2, StreaminformationRegion::ATTRIBUTE_TIME, 2.0);
    const double times[5] = { 1.0, 1.5, 2.0, 2.5, 3.0 };
    const double xi[3] = { 0.5, 0.5, 0.5 };
    const double xExpected[5][3] = {
        { 0.50, 0.50, 0.50 },
        { 0.75, 0.75, 0.75 },
        { 1.00, 1.00, 1.00 },
        { 1.25, 1.25, 1.25 },
        { 1.50, 1.50, 1.50 }
    };
    double xOut[3];
    // test reading twice
    const double tol = 1.0E-7;
    for (int i = 0; i < 2; ++i)
    {
        EXPECT_EQ(RESULT_OK, zinc.root_region.read(sir));
        Field coordinates = zinc.fm.findFieldByName("coordinates");
        EXPECT_TRUE(coordinates.isValid());
        Mesh mesh3d = zinc.fm.findMeshByDimension(3);
        EXPECT_TRUE(mesh3d.isValid());
        Element element1 = mesh3d.findElementByIdentifier(1);
        EXPECT_TRUE(element1.isValid());
        Fieldcache cache = zinc.fm.createFieldcache();
        EXPECT_TRUE(mesh3d.isValid());
        EXPECT_EQ(RESULT_OK, cache.setMeshLocation(element1, 3, xi));
        EXPECT_EQ(RESULT_OK, cache.setTime(times[i]));
        EXPECT_EQ(RESULT_OK, coordinates.evaluateReal(cache, 3, xOut));
        EXPECT_NEAR(xExpected[i][0], xOut[0], tol);
        EXPECT_NEAR(xExpected[i][1], xOut[1], tol);
        EXPECT_NEAR(xExpected[i][2], xOut[2], tol);
    }
}

namespace {

void checkAllShapesElementConstantModel(Fieldmodule& fm, double factor)
{
    Field coordinates = fm.findFieldByName("coordinates");
    EXPECT_TRUE(coordinates.isValid());
    EXPECT_EQ(3, coordinates.getNumberOfComponents());
    EXPECT_TRUE(coordinates.isTypeCoordinate());

    Field temperature = fm.findFieldByName("temperature");
    EXPECT_TRUE(temperature.isValid());
    EXPECT_EQ(1, temperature.getNumberOfComponents());
    EXPECT_FALSE(temperature.isTypeCoordinate());

    Mesh mesh3d = fm.findMeshByDimension(3);
    EXPECT_EQ(6, mesh3d.getSize());
    Mesh mesh2d = fm.findMeshByDimension(2);
    EXPECT_EQ(24, mesh2d.getSize());
    Mesh mesh1d = fm.findMeshByDimension(1);
    EXPECT_EQ(33, mesh1d.getSize());
    Nodeset nodes = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
    EXPECT_EQ(16, nodes.getSize());

    const Element::ShapeType expectedShapeTypes[6] = {
        Element::SHAPE_TYPE_WEDGE12,
        Element::SHAPE_TYPE_WEDGE12,
        Element::SHAPE_TYPE_WEDGE13,
        Element::SHAPE_TYPE_WEDGE23,
        Element::SHAPE_TYPE_CUBE,
        Element::SHAPE_TYPE_TETRAHEDRON
    };
    const double expectedTemperature[6] = { 48.0, 42.0, 19.0, 37.4, 11.5, 30.3 };
    double temperatureOut;
    Fieldcache cache = fm.createFieldcache();
    EXPECT_TRUE(cache.isValid());
    for (int i = 0; i < 6; ++i)
    {
        Element element = mesh3d.findElementByIdentifier(i + 1);
        EXPECT_TRUE(element.isValid());
        EXPECT_EQ(expectedShapeTypes[i], element.getShapeType());
        EXPECT_EQ(RESULT_OK, cache.setElement(element));
        EXPECT_EQ(RESULT_OK, temperature.evaluateReal(cache, 3, &temperatureOut));
        EXPECT_NEAR(factor*expectedTemperature[i], temperatureOut, 1.0E-6);
        const double temperatureIn = expectedTemperature[i]*2.0;
        EXPECT_EQ(RESULT_OK, temperature.assignReal(cache, 3, &temperatureIn));
        EXPECT_EQ(RESULT_OK, temperature.evaluateReal(cache, 3, &temperatureOut));
        EXPECT_NEAR(temperatureIn, temperatureOut, 1.0E-6);
    }
}

}

TEST(FieldIO, allShapesElementConstant)
{
    ZincTestSetupCpp zinc;

    EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(
        resourcePath("fieldmodule/allshapes.ex3").c_str()));
    EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(
        resourcePath("fieldio/allshapes_element_constant.ex2").c_str()));
    checkAllShapesElementConstantModel(zinc.fm, 1.0);

    std::string outFile = manageOutputFolderFieldML.getPath("/allshapes_element_constant.ex2");
    // test writing and re-reading in EX2 format
    EXPECT_EQ(OK, zinc.root_region.writeFile(outFile.c_str()));
    Region testRegion1 = zinc.root_region.createChild("test1");
    EXPECT_EQ(OK, testRegion1.readFile(outFile.c_str()));
    Fieldmodule testFm1 = testRegion1.getFieldmodule();
    checkAllShapesElementConstantModel(testFm1, 2.0);
}
