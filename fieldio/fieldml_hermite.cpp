/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>
#include <cmath>
#include <vector>

#include <zinc/field.hpp>
#include <zinc/fieldarithmeticoperators.hpp>
#include <zinc/fieldcache.hpp>
#include <zinc/fieldconstant.hpp>
#include <zinc/fieldfiniteelement.hpp>
#include <zinc/fieldgroup.hpp>
#include <zinc/fieldlogicaloperators.hpp>
#include <zinc/fieldmeshoperators.hpp>
#include <zinc/fieldmodule.hpp>
#include <zinc/fieldsubobjectgroup.hpp>
#include <zinc/region.hpp>
#include <zinc/streamregion.hpp>

#include "utilities/zinctestsetupcpp.hpp"
#include "utilities/fileio.hpp"

#include "test_resources.h"

#define FIELDML_OUTPUT_FOLDER "fieldmltest"

namespace {
ManageOutputFolder manageOutputFolderFieldML(FIELDML_OUTPUT_FOLDER);
}

namespace {

void check_twohermitecubes_noscalefactors_model(Fieldmodule& fm)
{
	int result;
	Field coordinates = fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	EXPECT_EQ(3, coordinates.getNumberOfComponents());
	EXPECT_TRUE(coordinates.isTypeCoordinate());
	Field pressure = fm.findFieldByName("pressure");
	EXPECT_TRUE(pressure.isValid());
	EXPECT_EQ(1, pressure.getNumberOfComponents());

	EXPECT_EQ(OK, result = fm.defineAllFaces());
	Mesh mesh3d = fm.findMeshByDimension(3);
	const int elementsCount = mesh3d.getSize();
	EXPECT_EQ(2, elementsCount);
	Mesh mesh2d = fm.findMeshByDimension(2);
	EXPECT_EQ(11, mesh2d.getSize());
	Mesh mesh1d = fm.findMeshByDimension(1);
	EXPECT_EQ(20, mesh1d.getSize());
	Nodeset nodes = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_EQ(12, nodes.getSize());
	Fieldcache cache = fm.createFieldcache();
	const int divisions = 3;
	const double divisionSize = 1.0 / static_cast<double>(divisions);
	double coordinatesOut[3], pressureOut, xi[3];
	for (int e = 1; e <= elementsCount; ++e)
	{
		Element element = mesh3d.findElementByIdentifier(e);
		EXPECT_TRUE(element.isValid());
		Element::ShapeType shapeType = element.getShapeType();
		EXPECT_EQ(Element::SHAPE_TYPE_CUBE, shapeType);
		double xOffset = static_cast<double>(e - 1);
		double pressureOffset = 80000.0 + 10000.0*xOffset;
		for (int k = 0; k <= divisions; ++k)
		{
			xi[2] = k*divisionSize;
			for (int j = 0; j <= divisions; ++j)
			{
				xi[1] = j*divisionSize;
				for (int i = 0; i <= divisions; ++i)
				{
					xi[0] = i*divisionSize;
					EXPECT_EQ(OK, result = cache.setMeshLocation(element, 3, xi));
					EXPECT_EQ(OK, result = coordinates.evaluateReal(cache, 3, coordinatesOut));
					EXPECT_DOUBLE_EQ(xOffset + divisionSize*i, coordinatesOut[0]);
					EXPECT_DOUBLE_EQ(divisionSize*j, coordinatesOut[1]);
					EXPECT_DOUBLE_EQ(divisionSize*k, coordinatesOut[2]);
					EXPECT_EQ(OK, result = pressure.evaluateReal(cache, 1, &pressureOut));
					EXPECT_DOUBLE_EQ(pressureOffset + 10000.0*divisionSize*i, pressureOut);
				}
			}
		}
	}

	Field isExterior = fm.createFieldIsExterior();
	EXPECT_TRUE(isExterior.isValid());
	FieldElementGroup exteriorFacesGroup = fm.createFieldElementGroup(mesh2d);
	EXPECT_TRUE(exteriorFacesGroup.isValid());
	EXPECT_EQ(OK, result = exteriorFacesGroup.setName("exterior"));
	MeshGroup exteriorFacesMeshGroup = exteriorFacesGroup.getMeshGroup();
	EXPECT_TRUE(exteriorFacesMeshGroup.isValid());
	EXPECT_EQ(OK, result = exteriorFacesMeshGroup.addElementsConditional(isExterior));
	EXPECT_EQ(10, exteriorFacesMeshGroup.getSize());
}

}

// a model consisting of two cubes with tricubic Hermite coordinates and trilinear pressure.
TEST(ZincRegion, fieldml_twohermitecubes_noscalefactors)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDIO_EX_TWOHERMITECUBES_NOSCALEFACTORS_RESOURCE)));
	check_twohermitecubes_noscalefactors_model(zinc.fm);

	// test writing and re-reading into different region
	EXPECT_EQ(OK, result = zinc.root_region.writeFile(FIELDML_OUTPUT_FOLDER "/twohermitecubes_noscalefactors.fieldml"));
	Region testRegion = zinc.root_region.createChild("test");
	EXPECT_EQ(OK, result = testRegion.readFile(FIELDML_OUTPUT_FOLDER "/twohermitecubes_noscalefactors.fieldml"));
	Fieldmodule testFm = testRegion.getFieldmodule();
	check_twohermitecubes_noscalefactors_model(testFm);
}

namespace {

void check_figure8_model(Fieldmodule& fm)
{
	int result;
	Field coordinates = fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	EXPECT_EQ(3, coordinates.getNumberOfComponents());
	EXPECT_TRUE(coordinates.isTypeCoordinate());

	EXPECT_EQ(OK, result = fm.defineAllFaces());
	Mesh mesh3d = fm.findMeshByDimension(3);
	const int elementsCount = mesh3d.getSize();
	EXPECT_EQ(7, elementsCount);
	Mesh mesh2d = fm.findMeshByDimension(2);
	EXPECT_EQ(34, mesh2d.getSize());
	Mesh mesh1d = fm.findMeshByDimension(1);
	EXPECT_EQ(52, mesh1d.getSize());
	Nodeset nodes = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_EQ(24, nodes.getSize());
	for (int e = 1; e <= elementsCount; ++e)
	{
		Element element = mesh3d.findElementByIdentifier(e);
		EXPECT_TRUE(element.isValid());
		Element::ShapeType shapeType = element.getShapeType();
		EXPECT_EQ(Element::SHAPE_TYPE_CUBE, shapeType);
	}

	FieldGroup exteriorGroup = fm.createFieldGroup();
	EXPECT_TRUE(exteriorGroup.isValid());
	EXPECT_EQ(OK, result = exteriorGroup.setName("exterior"));
	EXPECT_EQ(OK, result = exteriorGroup.setSubelementHandlingMode(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL));

	FieldElementGroup exteriorFacesGroup = exteriorGroup.getFieldElementGroup(mesh2d);
	EXPECT_FALSE(exteriorFacesGroup.isValid());
	exteriorFacesGroup = exteriorGroup.createFieldElementGroup(mesh2d);
	EXPECT_TRUE(exteriorFacesGroup.isValid());
	FieldIsExterior isExterior = fm.createFieldIsExterior();
	EXPECT_TRUE(isExterior.isValid());
	MeshGroup exteriorFacesMeshGroup = exteriorFacesGroup.getMeshGroup();
	EXPECT_EQ(OK, result = exteriorFacesMeshGroup.addElementsConditional(isExterior));
	EXPECT_EQ(26, exteriorFacesMeshGroup.getSize());

	FieldElementGroup exteriorLinesGroup = exteriorGroup.getFieldElementGroup(mesh1d);
	EXPECT_TRUE(exteriorLinesGroup.isValid());
	MeshGroup exteriorLinesMeshGroup = exteriorLinesGroup.getMeshGroup();
	EXPECT_EQ(52, exteriorLinesMeshGroup.getSize());

	const double valueOne = 1.0;
	const int numberOfVolumePoints = 3;
	Field one = fm.createFieldConstant(1, &valueOne);
	FieldMeshIntegral volume = fm.createFieldMeshIntegral(one, coordinates, mesh3d);
	EXPECT_EQ(OK, result = volume.setNumbersOfPoints(1, &numberOfVolumePoints));
	FieldMeshIntegral surfaceArea = fm.createFieldMeshIntegral(one, coordinates, exteriorFacesMeshGroup);
	// more surface Gauss points gives greater accuracy due to additional nonlinearity
	// of sqrt function in computing dA from 3-dimensional coordinates.
	const int numberOfSurfacePoints = 4;
	EXPECT_TRUE(surfaceArea.isValid());
	EXPECT_EQ(OK, result = surfaceArea.setNumbersOfPoints(1, &numberOfSurfacePoints));

	Fieldcache cache = fm.createFieldcache();
	double outVolume;
	EXPECT_EQ(OK, result = volume.evaluateReal(cache, 1, &outVolume));
	EXPECT_NEAR(31.445204155883715, outVolume, 1.0E-6);
	double outSurfaceArea;
	EXPECT_EQ(OK, result = surfaceArea.evaluateReal(cache, 1, &outSurfaceArea));
	EXPECT_NEAR(87.495697863403507, outSurfaceArea, 0.5E-5);
}

}

// a tricubic Hermite model, 7 elements in a Figure 8 shape. Uses versions.
TEST(ZincRegion, fieldml_figure8)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDIO_EX_HERMITE_FIGURE8_RESOURCE)));
	check_figure8_model(zinc.fm);

#if 0
	// test writing and re-reading into different region
	Region testRegion1 = zinc.root_region.createChild("test1");
	EXPECT_EQ(OK, result = testRegion1.readFile(FIELDML_OUTPUT_FOLDER "/figure8.fieldml"));
	Fieldmodule testFm1 = testRegion1.getFieldmodule();
	check_figure8_model(testFm1);
#endif

	// test writing and re-reading EX format, via a memory buffer
	StreaminformationRegion sir = zinc.root_region.createStreaminformationRegion();
	EXPECT_TRUE(sir.isValid());
	EXPECT_EQ(OK, result = sir.setFileFormat(StreaminformationRegion::FILE_FORMAT_EX));
	StreamresourceMemory resource = sir.createStreamresourceMemory();
	EXPECT_TRUE(resource.isValid());
	EXPECT_EQ(OK, result = zinc.root_region.write(sir));
	void *buffer;
	unsigned int bufferSize;
	EXPECT_EQ(OK, result = resource.getBuffer(&buffer, &bufferSize));

	Region testRegion2 = zinc.root_region.createChild("test2");
	EXPECT_TRUE(testRegion2.isValid());
	StreaminformationRegion sir2 = testRegion2.createStreaminformationRegion();
	EXPECT_TRUE(sir2.isValid());
	EXPECT_EQ(OK, result = sir2.setFileFormat(StreaminformationRegion::FILE_FORMAT_EX));
	StreamresourceMemory resource2 = sir2.createStreamresourceMemoryBuffer(buffer, bufferSize);
	EXPECT_TRUE(resource2.isValid());
	EXPECT_EQ(OK, result = testRegion2.read(sir2));
	Fieldmodule testFm2 = testRegion2.getFieldmodule();
	check_figure8_model(testFm2);
}

namespace {

class Bifurcation
{
	Fieldmodule& fm;
	int maxGeneration;
	// number nodes and elements by generation otherwise EX file is huge
	std::vector<int> nextNodeIndentifier;
	std::vector<int> nextElementIdentifier;
	FieldFiniteElement coordinates;
	FieldFiniteElement radius;
	Mesh mesh1d;
	Nodeset nodes;
	Nodetemplate nodetemplate1, nodetemplate3;
	Elementtemplate elementtemplate1, elementtemplate2, elementtemplate3;
	Fieldcache fieldcache;

public:
	Bifurcation(Fieldmodule& fm, int maxGeneration);

	void addElement(int generation, const Node& node1,
		const double *coordinates1, const double *direction1, double radius1, int version1,
		const double *coordinates2);
};

Bifurcation::Bifurcation(Fieldmodule& fmIn, int maxGenerationIn) :
	fm(fmIn),
	maxGeneration(maxGenerationIn),
	nextNodeIndentifier(maxGenerationIn + 1),
	nextElementIdentifier(maxGenerationIn + 1)
{
	int generationSize = 1;
	for (int g = 1; g <= maxGenerationIn; ++g)
	{
		nextNodeIndentifier[g] = generationSize + 1;
		nextElementIdentifier[g] = generationSize;
		generationSize *= 2;
	}

	fm.beginChange();
	int result;

	coordinates = fm.createFieldFiniteElement(/*numberOfComponents*/3);
	EXPECT_TRUE(coordinates.isValid());
	EXPECT_EQ(OK, result = coordinates.setName("coordinates"));
	EXPECT_EQ(OK, result = coordinates.setTypeCoordinate(true));
	EXPECT_EQ(OK, result = coordinates.setManaged(true));
	EXPECT_EQ(OK, result = coordinates.setComponentName(1, "x"));
	EXPECT_EQ(OK, result = coordinates.setComponentName(2, "y"));
	EXPECT_EQ(OK, result = coordinates.setComponentName(3, "z"));

	radius = fm.createFieldFiniteElement(/*numberOfComponents*/1);
	EXPECT_TRUE(radius.isValid());
	EXPECT_EQ(OK, result = radius.setName("radius"));
	EXPECT_EQ(OK, result = radius.setManaged(true));

	nodes = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodes.isValid());

	nodetemplate1 = nodes.createNodetemplate();
	EXPECT_EQ(OK, result = nodetemplate1.defineField(coordinates));
	EXPECT_EQ(OK, result = nodetemplate1.setValueNumberOfVersions(coordinates, -1, Node::VALUE_LABEL_D_DS1, 1));
	EXPECT_EQ(OK, result = nodetemplate1.defineField(radius));
	nodetemplate3 = nodes.createNodetemplate();
	EXPECT_EQ(OK, result = nodetemplate3.defineField(coordinates));
	EXPECT_EQ(OK, result = nodetemplate3.setValueNumberOfVersions(coordinates, -1, Node::VALUE_LABEL_D_DS1, 3));
	EXPECT_EQ(OK, result = nodetemplate3.defineField(radius));
	EXPECT_EQ(OK, result = nodetemplate3.setValueNumberOfVersions(radius, -1, Node::VALUE_LABEL_VALUE, 3));

	mesh1d = fm.findMeshByDimension(1);
	EXPECT_TRUE(mesh1d.isValid());

	Elementbasis linearBasis = fm.createElementbasis(1, Elementbasis::FUNCTION_TYPE_LINEAR_LAGRANGE);
	EXPECT_TRUE(linearBasis.isValid());
	Elementbasis cubicHermiteBasis = fm.createElementbasis(1, Elementbasis::FUNCTION_TYPE_CUBIC_HERMITE);
	EXPECT_TRUE(cubicHermiteBasis.isValid());

	const int localNodeIndexes[2] = { 1, 2 };
	elementtemplate1 = mesh1d.createElementtemplate();
	EXPECT_EQ(OK, result = elementtemplate1.setElementShapeType(Element::SHAPE_TYPE_LINE));
	EXPECT_EQ(OK, result = elementtemplate1.setNumberOfNodes(2));
	EXPECT_EQ(OK, result = elementtemplate1.defineFieldSimpleNodal(coordinates, -1, cubicHermiteBasis, 2, localNodeIndexes));
	EXPECT_EQ(OK, result = elementtemplate1.setMapNodeValueLabel(coordinates, -1, /*node*/1, /*function*/2, Node::VALUE_LABEL_D_DS1));
	EXPECT_EQ(OK, result = elementtemplate1.setMapNodeVersion(coordinates, -1, /*node*/1, /*function*/2, 1));
	EXPECT_EQ(OK, result = elementtemplate1.defineFieldSimpleNodal(radius, -1, linearBasis, 2, localNodeIndexes));
	elementtemplate2 = mesh1d.createElementtemplate();
	EXPECT_EQ(OK, result = elementtemplate2.setElementShapeType(Element::SHAPE_TYPE_LINE));
	EXPECT_EQ(OK, result = elementtemplate2.setNumberOfNodes(2));
	EXPECT_EQ(OK, result = elementtemplate2.defineFieldSimpleNodal(coordinates, -1, cubicHermiteBasis, 2, localNodeIndexes));
	EXPECT_EQ(OK, result = elementtemplate2.setMapNodeVersion(coordinates, -1, /*node*/1, /*function*/2, 2));
	EXPECT_EQ(OK, result = elementtemplate2.defineFieldSimpleNodal(radius, -1, linearBasis, 2, localNodeIndexes));
	EXPECT_EQ(OK, result = elementtemplate2.setMapNodeVersion(radius, -1, /*node*/1, /*function*/1, 2));
	elementtemplate3 = mesh1d.createElementtemplate();
	EXPECT_EQ(OK, result = elementtemplate3.setElementShapeType(Element::SHAPE_TYPE_LINE));
	EXPECT_EQ(OK, result = elementtemplate3.setNumberOfNodes(2));
	EXPECT_EQ(OK, result = elementtemplate3.defineFieldSimpleNodal(coordinates, -1, cubicHermiteBasis, 2, localNodeIndexes));
	EXPECT_EQ(OK, result = elementtemplate3.setMapNodeVersion(coordinates, -1, /*node*/1, /*function*/2, 3));
	EXPECT_EQ(OK, result = elementtemplate3.defineFieldSimpleNodal(radius, -1, linearBasis, 2, localNodeIndexes));
	EXPECT_EQ(OK, result = elementtemplate3.setMapNodeVersion(radius, -1, /*node*/1, /*function*/1, 3));

	fieldcache = fm.createFieldcache();

	int generation = 1;
	const int nodeIdentifier = 1;
	Node node1 = nodes.createNode(nodeIdentifier, nodetemplate1);
	EXPECT_TRUE(node1.isValid());
	const double coordinates1[3] = { 0.0, 0.0, 0.0 };
	const double direction1[3] = { 0.0, 0.0, 1.0 };
	const double radius = 0.1;
	addElement(generation, node1, coordinates1, direction1, radius, /*version1*/1, direction1);
	fm.endChange();
}

void Bifurcation::addElement(int generation, const Node& node1,
	const double *coordinates1, const double *direction1, double radius1, int version1,
	const double *direction2)
{
	EXPECT_EQ(OK, fieldcache.setNode(node1));
	const double zeroCoordinates[3] = { 0.0, 0.0, 0.0 };
	EXPECT_EQ(OK, coordinates.setNodeParameters(fieldcache, /*component*/-1, Node::VALUE_LABEL_VALUE, version1, 3,
		(version1 == 1) ? coordinates1 : zeroCoordinates));
	EXPECT_EQ(OK, coordinates.setNodeParameters(fieldcache, /*component*/-1, Node::VALUE_LABEL_D_DS1, version1, 3, direction1));
	EXPECT_EQ(OK, radius.setNodeParameters(fieldcache, /*component*/-1, Node::VALUE_LABEL_VALUE, version1, 1, &radius1));

	double coordinates2[3];
	for (int i = 0; i < 3; ++i)
		coordinates2[i] = coordinates1[i] + 0.5*direction1[i] + 0.4*direction2[i];
	double radius2 = radius1*0.8;
	const int nodeIdentifier = (nextNodeIndentifier[generation])++;
	Node node2 = this->nodes.createNode(nodeIdentifier, (generation < maxGeneration) ? nodetemplate3 : nodetemplate1);
	EXPECT_TRUE(node2.isValid());
	EXPECT_EQ(OK, fieldcache.setNode(node2));
	EXPECT_EQ(OK, coordinates.setNodeParameters(fieldcache, /*component*/-1, Node::VALUE_LABEL_VALUE, /*version*/1, 3, coordinates2));
	EXPECT_EQ(OK, coordinates.setNodeParameters(fieldcache, /*component*/-1, Node::VALUE_LABEL_D_DS1, /*version*/1, 3, direction2));
	EXPECT_EQ(OK, radius.setNodeParameters(fieldcache, /*component*/-1, Node::VALUE_LABEL_VALUE, /*version*/1, 1, &radius2));

	Elementtemplate elementtemplate =
		(version1 == 1) ? elementtemplate1 :
		(version1 == 2) ? elementtemplate2 : elementtemplate3;
	EXPECT_EQ(OK, elementtemplate.setNode(1, node1));
	EXPECT_EQ(OK, elementtemplate.setNode(2, node2));
	int elementIdentifier = nextElementIdentifier[generation];
	// keep different versions together to reduce EX file size
	if (version1 == 3)
		elementIdentifier += (1 << (generation - 2)) - 1;
	else
		++(nextElementIdentifier[generation]);
	Element element = mesh1d.createElement(elementIdentifier, elementtemplate);
	EXPECT_TRUE(element.isValid());

	if (generation < maxGeneration)
	{
		double normal[3] =
		{
			direction1[1]*direction2[2] - direction1[2]*direction2[1],
			direction1[2]*direction2[0] - direction1[0]*direction2[2],
			direction1[0]*direction2[1] - direction1[1]*direction2[0]
		};
		if (generation == 1)
		{
			normal[0] = 0.0;
			normal[1] = 1.0;
			normal[2] = 0.0;
		}
		else
		{
			double normalLength = 0.0;
			double directionLength = 0.0;
			for (int i = 0; i < 3; ++i)
			{
				normalLength += normal[i]*normal[i];
				directionLength += direction2[i]*direction2[i];
			}
			double factor = sqrt(directionLength) / sqrt(normalLength);
			for (int i = 0; i < 3; ++i)
				normal[i] *= factor;
		}
		double direction1Child1[3];
		double direction1Child2[3];
		double direction2Child1[3];
		double direction2Child2[3];
		for (int i = 0; i < 3; ++i)
		{
			direction1Child1[i] = 0.6*direction2[i] - 0.2*normal[i];
			direction1Child2[i] = 0.6*direction2[i] + 0.2*normal[i];
			direction2Child1[i] = 0.4*direction2[i] - 0.65*normal[i];
			direction2Child2[i] = 0.4*direction2[i] + 0.65*normal[i];
		}
		double radiusChild = sqrt(0.5*radius2*radius2)/0.8;
		addElement(generation + 1, node2, coordinates2, direction1Child1, radiusChild, /*version1*/2, direction2Child1);
		addElement(generation + 1, node2, coordinates2, direction1Child2, radiusChild, /*version1*/3, direction2Child2);
	}
}

/*
 * To test non-contiguous indexing, offset node and element identifiers
 * to be in idBlockSize blocks offset by idBlockSpan, starting at idFirst
 * e.g. idFirst = 1, idBlockSize = 5, idBlockSpan = 7 gives:
 * 1 2 3 4 5 8 9 10 11 12 15 16 ...
 * Note: only call once as expects identifiers to initially be consecutive from 1
 */
void offset_identifiers(Fieldmodule& fm, int idFirst, int idBlockSize, int idBlockSpan)
{
	fm.beginChange();
	Mesh mesh1d = fm.findMeshByDimension(1);
	int elementsCount = mesh1d.getSize();
	for (int e = elementsCount - 1; e >= 0; --e)
	{
		Element element = mesh1d.findElementByIdentifier(e + 1);
		EXPECT_TRUE(element.isValid());
		const int identifier = (e / idBlockSize)*idBlockSpan + (e % idBlockSize) + idFirst;
		EXPECT_EQ(OK, element.setIdentifier(identifier));
	}
	Nodeset nodes = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	int nodesCount = nodes.getSize();
	for (int n = nodesCount - 1; n >= 0; --n)
	{
		Node node = nodes.findNodeByIdentifier(n + 1);
		EXPECT_TRUE(node.isValid());
		const int identifier = (n / idBlockSize)*idBlockSpan + (n % idBlockSize) + idFirst;
		EXPECT_EQ(OK, node.setIdentifier(identifier));
	}
	fm.endChange();
}

/*
 * To test non-contiguous indexing, node and element identifiers are
 * expected to be in idBlockSize blocks offset by idBlockSpan
 * e.g. idBlockSize = 5, idBlockSpan = 7 gives:
 * 1 2 3 4 5 8 9 10 11 12 15 16 ...
 */
void check_bifurcation(Fieldmodule& fm, int idFirst = 1, int idBlockSize = 1, int idBlockSpan = 1)
{
	int result;
	Field coordinates = fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	EXPECT_EQ(3, coordinates.getNumberOfComponents());
	EXPECT_TRUE(coordinates.isTypeCoordinate());
	Field radius = fm.findFieldByName("radius");
	EXPECT_TRUE(radius.isValid());
	EXPECT_EQ(1, radius.getNumberOfComponents());

	Mesh mesh1d = fm.findMeshByDimension(1);
	int elementsCount = mesh1d.getSize();
	EXPECT_EQ(255, elementsCount); // 8 generations
	//EXPECT_EQ(1023, elementsCount); // 10 generations
	for (int e = 0; e < elementsCount; ++e)
	{
		const int identifier = (e / idBlockSize)*idBlockSpan + (e % idBlockSize) + idFirst;
		Element element = mesh1d.findElementByIdentifier(identifier);
		EXPECT_TRUE(element.isValid());
		Element::ShapeType shapeType = element.getShapeType();
		EXPECT_EQ(Element::SHAPE_TYPE_LINE, shapeType);
	}
	Nodeset nodes = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	int nodesCount = nodes.getSize();
	EXPECT_EQ(256, nodesCount); // 8 generations
	//EXPECT_EQ(1024, nodesCount); // 10 generations
	for (int n = 0; n < nodesCount; ++n)
	{
		const int identifier = (n / idBlockSize)*idBlockSpan + (n % idBlockSize) + idFirst;
		Node node = nodes.findNodeByIdentifier(identifier);
		EXPECT_TRUE(node.isValid());
	}

	// integrate to get volume of vessels
	const double piConstant = 3.14159265358979323846;
	FieldConstant pi = fm.createFieldConstant(1, &piConstant);
	EXPECT_TRUE(pi.isValid());
	Field area = pi*radius*radius;
	EXPECT_TRUE(area.isValid());
	FieldMeshIntegral volume = fm.createFieldMeshIntegral(area, coordinates, mesh1d);
	EXPECT_TRUE(volume.isValid());
	const int numberOfGaussPoints = 4;
	EXPECT_EQ(OK, result = volume.setNumbersOfPoints(1, &numberOfGaussPoints));

	Fieldcache fieldcache = fm.createFieldcache();
	EXPECT_TRUE(fieldcache.isValid());
	double volumeOut;
	EXPECT_EQ(OK, result = volume.evaluateReal(fieldcache, 1, &volumeOut));
	EXPECT_NEAR(volumeOut, 0.077529809577001507, 1E-12); // 8 generations
	//EXPECT_NEAR(volumeOut, 0.081574031223926702, 1E-12); // 10 generations
}

}

// 1D bifurcating network using Hermite for curving geometry
// and versions of coordinate derivatives and radius fields to give different
// directions and sizes at bifurations.
TEST(ZincRegion, bifurcation)
{
	ZincTestSetupCpp zinc;
	int result;

	Bifurcation model(zinc.fm, /*maxGenerations*/8);
	check_bifurcation(zinc.fm);

	// test writing and re-reading in EX format
	EXPECT_EQ(OK, result = zinc.root_region.writeFile(FIELDML_OUTPUT_FOLDER "/bifurcation.exregion"));
	Region testRegion1 = zinc.root_region.createChild("test1");
	EXPECT_EQ(OK, result = testRegion1.readFile(FIELDML_OUTPUT_FOLDER "/bifurcation.exregion"));
	Fieldmodule testFm1 = testRegion1.getFieldmodule();
	check_bifurcation(testFm1);

	// test writing and re-reading in FieldML format
	EXPECT_EQ(OK, result = zinc.root_region.writeFile(FIELDML_OUTPUT_FOLDER "/bifurcation.fieldml"));
	Region testRegion2 = zinc.root_region.createChild("test2");
	EXPECT_EQ(OK, result = testRegion2.readFile(FIELDML_OUTPUT_FOLDER "/bifurcation.fieldml"));
	Fieldmodule testFm2 = testRegion2.getFieldmodule();
	check_bifurcation(testFm2);

	// test writing and re-reading in FieldML format, with non-contiguous
	// element and node identifiers to test different code used in that case
	offset_identifiers(testFm2, /*idFirst*/1, /*idBlockSize*/6, /*idBlockSpan*/8);
	EXPECT_EQ(OK, result = testRegion2.writeFile(FIELDML_OUTPUT_FOLDER "/bifurcation_noncontiguous.fieldml"));
	Region testRegion3 = zinc.root_region.createChild("test3");
	EXPECT_EQ(OK, result = testRegion3.readFile(FIELDML_OUTPUT_FOLDER "/bifurcation_noncontiguous.fieldml"));
	Fieldmodule testFm3 = testRegion3.getFieldmodule();
	check_bifurcation(testFm3, /*idFirst*/1, /*idBlockSize*/6, /*idBlockSpan*/8);
}
