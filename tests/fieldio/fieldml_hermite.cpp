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

#include <opencmiss/zinc/field.hpp>
#include <opencmiss/zinc/fieldarithmeticoperators.hpp>
#include <opencmiss/zinc/fieldcache.hpp>
#include <opencmiss/zinc/fieldconstant.hpp>
#include <opencmiss/zinc/fieldcoordinatetransformation.hpp>
#include <opencmiss/zinc/fieldfiniteelement.hpp>
#include <opencmiss/zinc/fieldgroup.hpp>
#include <opencmiss/zinc/fieldlogicaloperators.hpp>
#include <opencmiss/zinc/fieldmeshoperators.hpp>
#include <opencmiss/zinc/fieldmodule.hpp>
#include <opencmiss/zinc/fieldsubobjectgroup.hpp>
#include <opencmiss/zinc/region.hpp>
#include <opencmiss/zinc/streamregion.hpp>

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
	const void *buffer;
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

	// test writing and re-reading EX format, via file
	EXPECT_EQ(OK, result = zinc.root_region.writeFile(FIELDML_OUTPUT_FOLDER "/figure8.ex2"));
	Region testRegion3 = zinc.root_region.createChild("test3");
	EXPECT_TRUE(testRegion3.isValid());
	EXPECT_EQ(OK, result = testRegion3.readFile(FIELDML_OUTPUT_FOLDER "/figure8.ex2"));
	Fieldmodule testFm3 = testRegion3.getFieldmodule();
	check_figure8_model(testFm3);
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
	EXPECT_EQ(OK, result = nodetemplate3.setValueNumberOfVersions(radius, -1, Node::VALUE_LABEL_VALUE, 2));

	mesh1d = fm.findMeshByDimension(1);
	EXPECT_TRUE(mesh1d.isValid());

	Elementbasis cubicHermiteBasis = fm.createElementbasis(1, Elementbasis::FUNCTION_TYPE_CUBIC_HERMITE);
	EXPECT_TRUE(cubicHermiteBasis.isValid());
	Elementfieldtemplate eftHermite1 = mesh1d.createElementfieldtemplate(cubicHermiteBasis);
	EXPECT_TRUE(eftHermite1.isValid());
	Elementfieldtemplate eftHermite2 = mesh1d.createElementfieldtemplate(cubicHermiteBasis);
	EXPECT_TRUE(eftHermite2.isValid());
	EXPECT_EQ(OK, eftHermite2.setTermNodeParameter(/*function*/2, /*term*/1, /*localNodeIndex*/1, Node::VALUE_LABEL_D_DS1, /*version*/2));
	Elementfieldtemplate eftHermite3 = mesh1d.createElementfieldtemplate(cubicHermiteBasis);
	EXPECT_TRUE(eftHermite3.isValid());
	EXPECT_EQ(OK, eftHermite3.setTermNodeParameter(/*function*/2, /*term*/1, /*localNodeIndex*/1, Node::VALUE_LABEL_D_DS1, /*version*/3));

	Elementbasis linearBasis = fm.createElementbasis(1, Elementbasis::FUNCTION_TYPE_LINEAR_LAGRANGE);
	EXPECT_TRUE(linearBasis.isValid());
	Elementfieldtemplate eftLinear1 = mesh1d.createElementfieldtemplate(linearBasis);
	EXPECT_TRUE(eftLinear1.isValid());
	Elementfieldtemplate eftLinear2 = mesh1d.createElementfieldtemplate(linearBasis);
	EXPECT_TRUE(eftLinear2.isValid());
	EXPECT_EQ(OK, eftLinear2.setTermNodeParameter(/*function*/1, /*term*/1, /*localNodeIndex*/1, Node::VALUE_LABEL_VALUE, /*version*/2));

	this->elementtemplate1 = mesh1d.createElementtemplate();
	EXPECT_EQ(OK, this->elementtemplate1.setElementShapeType(Element::SHAPE_TYPE_LINE));
	EXPECT_EQ(OK, this->elementtemplate1.defineField(coordinates, /*componentNumber=all*/-1, eftHermite1));
	EXPECT_EQ(OK, this->elementtemplate1.defineField(radius, /*componentNumber=all*/-1, eftLinear1));
	this->elementtemplate2 = mesh1d.createElementtemplate();
	EXPECT_EQ(OK, this->elementtemplate2.setElementShapeType(Element::SHAPE_TYPE_LINE));
	EXPECT_EQ(OK, this->elementtemplate2.defineField(coordinates, /*componentNumber=all*/-1, eftHermite2));
	EXPECT_EQ(OK, this->elementtemplate2.defineField(radius, /*componentNumber=all*/-1, eftLinear2));
	this->elementtemplate3 = mesh1d.createElementtemplate();
	EXPECT_EQ(OK, this->elementtemplate3.setElementShapeType(Element::SHAPE_TYPE_LINE));
	EXPECT_EQ(OK, this->elementtemplate3.defineField(coordinates, /*componentNumber=all*/-1, eftHermite3));
	EXPECT_EQ(OK, this->elementtemplate3.defineField(radius, /*componentNumber=all*/-1, eftLinear2));

	this->fieldcache = fm.createFieldcache();

	int generation = 1;
	const int nodeIdentifier = 1;
	Node node1 = nodes.createNode(nodeIdentifier, nodetemplate1);
	EXPECT_TRUE(node1.isValid());
	const double coordinates1[3] = { 0.0, 0.0, 0.0 };
	const double direction1[3] = { 0.0, 0.0, 1.0 };
	const double radius1 = 0.1;
	EXPECT_EQ(OK, fieldcache.setNode(node1));
	EXPECT_EQ(OK, coordinates.setNodeParameters(fieldcache, /*component*/-1, Node::VALUE_LABEL_VALUE, /*version*/1, 3, coordinates1));
	EXPECT_EQ(OK, coordinates.setNodeParameters(fieldcache, /*component*/-1, Node::VALUE_LABEL_D_DS1, /*version*/1, 3, direction1));
	EXPECT_EQ(OK, radius.setNodeParameters(fieldcache, /*component*/-1, Node::VALUE_LABEL_VALUE, /*version*/1, 1, &radius1));

	addElement(generation, node1, coordinates1, direction1, radius1, /*version1*/1, direction1);
	fm.endChange();
}

void Bifurcation::addElement(int generation, const Node& node1,
	const double *coordinates1, const double *direction1, double radius1, int version1,
	const double *direction2)
{
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
	int elementIdentifier = nextElementIdentifier[generation];
	// keep different versions together to reduce EX file size
	if (version1 == 3)
		elementIdentifier += (1 << (generation - 2)) - 1;
	else
		++(nextElementIdentifier[generation]);
	Element element = mesh1d.createElement(elementIdentifier, elementtemplate);
	EXPECT_TRUE(element.isValid());

	// each eft has independent local-to-global nodes so must set for each
	Elementfieldtemplate coordinatesEft = element.getElementfieldtemplate(coordinates, /*component*/-1);
	EXPECT_TRUE(coordinatesEft.isValid());
	element.setNode(coordinatesEft, 1, node1);
	element.setNode(coordinatesEft, 2, node2);
	Elementfieldtemplate radiusEft = element.getElementfieldtemplate(radius, /*component*/-1);
	EXPECT_TRUE(radiusEft.isValid());
	element.setNode(radiusEft, 1, node1);
	element.setNode(radiusEft, 2, node2);

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
		EXPECT_EQ(OK, coordinates.setNodeParameters(fieldcache, /*component*/-1, Node::VALUE_LABEL_D_DS1, /*version*/2, 3, direction1Child1));
		EXPECT_EQ(OK, coordinates.setNodeParameters(fieldcache, /*component*/-1, Node::VALUE_LABEL_D_DS1, /*version*/3, 3, direction1Child2));
		EXPECT_EQ(OK, radius.setNodeParameters(fieldcache, /*component*/-1, Node::VALUE_LABEL_VALUE, /*version*/2, 1, &radiusChild));
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
	EXPECT_EQ(OK, result = zinc.root_region.writeFile(FIELDML_OUTPUT_FOLDER "/bifurcation.ex2"));
	Region testRegion1 = zinc.root_region.createChild("test1");
	EXPECT_EQ(OK, result = testRegion1.readFile(FIELDML_OUTPUT_FOLDER "/bifurcation.ex2"));
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

namespace {

void check_prolate_heart_model(Fieldmodule& fm)
{
	int result;
	Field coordinates = fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	EXPECT_EQ(3, coordinates.getNumberOfComponents());
	EXPECT_TRUE(coordinates.isTypeCoordinate());
	Field fibres = fm.findFieldByName("fibres");
	EXPECT_TRUE(fibres.isValid());
	EXPECT_EQ(3, fibres.getNumberOfComponents());

	Mesh mesh3d = fm.findMeshByDimension(3);
	EXPECT_EQ(60, mesh3d.getSize());
	Mesh mesh2d = fm.findMeshByDimension(2);
	EXPECT_EQ(218, mesh2d.getSize());
	Mesh mesh1d = fm.findMeshByDimension(1);
	EXPECT_EQ(256, mesh1d.getSize());
	Nodeset nodes = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_EQ(99, nodes.getSize());

	// make the group of exterior faces
	Field isExterior = fm.createFieldIsExterior();
	EXPECT_TRUE(isExterior.isValid());
	FieldGroup exteriorGroup = fm.createFieldGroup();
	EXPECT_TRUE(exteriorGroup.isValid());
	EXPECT_EQ(RESULT_OK, result = exteriorGroup.setName("exterior"));
	FieldElementGroup exteriorFacesGroup = exteriorGroup.createFieldElementGroup(mesh2d);
	EXPECT_TRUE(exteriorFacesGroup.isValid());
	MeshGroup exteriorFacesMeshGroup = exteriorFacesGroup.getMeshGroup();
	EXPECT_TRUE(exteriorFacesMeshGroup.isValid());
	EXPECT_EQ(RESULT_OK, result = exteriorFacesMeshGroup.addElementsConditional(isExterior));
	EXPECT_EQ(96, exteriorFacesMeshGroup.getSize());

	Field rc_coordinates = fm.createFieldCoordinateTransformation(coordinates);
	EXPECT_TRUE(rc_coordinates.isValid());
	EXPECT_EQ(RESULT_OK, result = rc_coordinates.setCoordinateSystemType(Field::COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN));

	const double valueOne = 1.0;
	Field one = fm.createFieldConstant(1, &valueOne);
	FieldMeshIntegral volume = fm.createFieldMeshIntegral(one, rc_coordinates, mesh3d);
	EXPECT_TRUE(volume.isValid());
	const int numGaussPoints = 4;
	EXPECT_EQ(RESULT_OK, result = volume.setNumbersOfPoints(1, &numGaussPoints));
	FieldMeshIntegral surfaceArea = fm.createFieldMeshIntegral(one, rc_coordinates, exteriorFacesMeshGroup);
	EXPECT_TRUE(surfaceArea.isValid());
	EXPECT_EQ(RESULT_OK, result = surfaceArea.setNumbersOfPoints(1, &numGaussPoints));

	Fieldcache cache = fm.createFieldcache();
	double outVolume;
	EXPECT_EQ(RESULT_OK, result = volume.evaluateReal(cache, 1, &outVolume));
	EXPECT_NEAR(199205.30541176599, outVolume, 1.0E-1);
	double outSurfaceArea;
	EXPECT_EQ(RESULT_OK, result = surfaceArea.evaluateReal(cache, 1, &outSurfaceArea));
	EXPECT_NEAR(34589.128638176546, outSurfaceArea, 1.0E-1);
}

}

TEST(FieldIO, prolate_heart)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(RESULT_OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDIO_EX_PROLATE_HEART_RESOURCE)));
	check_prolate_heart_model(zinc.fm);

	// test writing and re-reading in FieldML format
	EXPECT_EQ(RESULT_OK, result = zinc.root_region.writeFile(FIELDML_OUTPUT_FOLDER "/prolate_heart.ex2"));
	Region testRegion1 = zinc.root_region.createChild("test1");
	EXPECT_EQ(RESULT_OK, result = testRegion1.readFile(FIELDML_OUTPUT_FOLDER "/prolate_heart.ex2"));
	Fieldmodule testFm1 = testRegion1.getFieldmodule();
	check_prolate_heart_model(testFm1);
}


namespace {

void check_hemisphere_model(Fieldmodule& fm)
{
	int result;
	Field coordinates = fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	EXPECT_EQ(3, coordinates.getNumberOfComponents());
	EXPECT_TRUE(coordinates.isTypeCoordinate());

	Mesh mesh2d = fm.findMeshByDimension(2);
	EXPECT_EQ(48, mesh2d.getSize());
	Mesh mesh1d = fm.findMeshByDimension(1);
	EXPECT_EQ(100, mesh1d.getSize());
	Nodeset nodes = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_EQ(53, nodes.getSize());

	const double valueOne = 1.0;
	Field one = fm.createFieldConstant(1, &valueOne);
	FieldMeshIntegral area = fm.createFieldMeshIntegral(one, coordinates, mesh2d);
	EXPECT_TRUE(area.isValid());
	const int numGaussPoints = 4;
	EXPECT_EQ(RESULT_OK, result = area.setNumbersOfPoints(1, &numGaussPoints));

	Fieldcache cache = fm.createFieldcache();
	double areaOut;
	EXPECT_EQ(RESULT_OK, result = area.evaluateReal(cache, 1, &areaOut));
	EXPECT_NEAR(9.5681385964832906, areaOut, 1.0E-7);
}

}

TEST(FieldIO, hemisphere)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(RESULT_OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDIO_EX_HEMISPHERE_RESOURCE)));
	check_hemisphere_model(zinc.fm);

	// test writing and re-reading in FieldML format
	EXPECT_EQ(RESULT_OK, result = zinc.root_region.writeFile(FIELDML_OUTPUT_FOLDER "/hemisphere.ex2"));
	Region testRegion1 = zinc.root_region.createChild("test1");
	EXPECT_EQ(RESULT_OK, result = testRegion1.readFile(FIELDML_OUTPUT_FOLDER "/hemisphere.ex2"));
	Fieldmodule testFm1 = testRegion1.getFieldmodule();
	check_hemisphere_model(testFm1);
}

namespace {

const double PI = 3.14159265358979323846;

void createHemisphereTube2d(Fieldmodule& fm, int elementsAroundCount, int elementsUpCount, int elementsTubeCount)
{
	int result;
	FieldFiniteElement coordinates = fm.createFieldFiniteElement(/*numberOfComponents*/3);
	EXPECT_TRUE(coordinates.isValid());
	EXPECT_EQ(RESULT_OK, result = coordinates.setName("coordinates"));
	EXPECT_EQ(RESULT_OK, result = coordinates.setTypeCoordinate(true));
	EXPECT_EQ(RESULT_OK, result = coordinates.setCoordinateSystemType(Field::COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN));
	EXPECT_EQ(RESULT_OK, result = coordinates.setManaged(true));
	EXPECT_EQ(RESULT_OK, result = coordinates.setComponentName(1, "x"));
	EXPECT_EQ(RESULT_OK, result = coordinates.setComponentName(2, "y"));
	EXPECT_EQ(RESULT_OK, result = coordinates.setComponentName(3, "z"));

	Nodeset nodes = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodes.isValid());

	Nodetemplate nodetemplate = nodes.createNodetemplate();
	EXPECT_TRUE(nodetemplate.isValid());
	EXPECT_EQ(RESULT_OK, result = nodetemplate.defineField(coordinates));
	EXPECT_EQ(RESULT_OK, result = nodetemplate.setValueNumberOfVersions(coordinates, -1, Node::VALUE_LABEL_VALUE, 1));
	EXPECT_EQ(RESULT_OK, result = nodetemplate.setValueNumberOfVersions(coordinates, -1, Node::VALUE_LABEL_D_DS1, 1));
	EXPECT_EQ(RESULT_OK, result = nodetemplate.setValueNumberOfVersions(coordinates, -1, Node::VALUE_LABEL_D_DS2, 1));
	EXPECT_EQ(RESULT_OK, result = nodetemplate.setValueNumberOfVersions(coordinates, -1, Node::VALUE_LABEL_D2_DS1DS2, 1));

	// can't use cross derivative at collapsed apex so use different node template there
	Nodetemplate nodetemplateApex = nodes.createNodetemplate();
	EXPECT_TRUE(nodetemplateApex.isValid());
	EXPECT_EQ(RESULT_OK, result = nodetemplateApex.defineField(coordinates));
	EXPECT_EQ(RESULT_OK, result = nodetemplateApex.setValueNumberOfVersions(coordinates, -1, Node::VALUE_LABEL_VALUE, 1));
	EXPECT_EQ(RESULT_OK, result = nodetemplateApex.setValueNumberOfVersions(coordinates, -1, Node::VALUE_LABEL_D_DS1, 1));
	EXPECT_EQ(RESULT_OK, result = nodetemplateApex.setValueNumberOfVersions(coordinates, -1, Node::VALUE_LABEL_D_DS2, 1));

	Elementbasis bicubicHermiteBasis = fm.createElementbasis(2, Elementbasis::FUNCTION_TYPE_CUBIC_HERMITE);
	EXPECT_TRUE(bicubicHermiteBasis.isValid());

	Mesh mesh = fm.findMeshByDimension(2);
	EXPECT_TRUE(mesh.isValid());

	// main element field template for regular elements away from apex is the default bicubic Hermite mapping
	Elementfieldtemplate eft = mesh.createElementfieldtemplate(bicubicHermiteBasis);
	EXPECT_TRUE(eft.isValid());
	EXPECT_EQ(Elementfieldtemplate::PARAMETER_MAPPING_MODE_NODE, eft.getParameterMappingMode());

	// apex element field template uses local general scale factors to use proportions of global derivatives
	// in a general linear map to give derivatives w.r.t. xi 2.
	// Note xi1 is around the apex, xi2 is away from the apex.
	// Note the eft needs to be edited around the apex as the local scale factor versions change.
	// parameters for basis function multiplying derivative w.r.t. xi1 and cross derivative
	// are zero at the apex, achieved by using 0 terms.
	Elementfieldtemplate eftApex = mesh.createElementfieldtemplate(bicubicHermiteBasis);
	EXPECT_TRUE(eftApex.isValid());
	EXPECT_EQ(RESULT_OK, eftApex.setNumberOfLocalNodes(3));
	EXPECT_EQ(3, eftApex.getNumberOfLocalNodes());
	EXPECT_EQ(RESULT_OK, eftApex.setNumberOfLocalScaleFactors(4));
	EXPECT_EQ(RESULT_OK, eftApex.setScaleFactorType(1, Elementfieldtemplate::SCALE_FACTOR_TYPE_NODE_GENERAL));
	EXPECT_EQ(RESULT_OK, eftApex.setScaleFactorIdentifier(1, 1));
	EXPECT_EQ(RESULT_OK, eftApex.setScaleFactorType(2, Elementfieldtemplate::SCALE_FACTOR_TYPE_NODE_GENERAL));
	EXPECT_EQ(RESULT_OK, eftApex.setScaleFactorIdentifier(2, 2));
	EXPECT_EQ(RESULT_OK, eftApex.setScaleFactorType(3, Elementfieldtemplate::SCALE_FACTOR_TYPE_NODE_GENERAL));
	EXPECT_EQ(RESULT_OK, eftApex.setScaleFactorIdentifier(3, 101));
	EXPECT_EQ(RESULT_OK, eftApex.setScaleFactorType(4, Elementfieldtemplate::SCALE_FACTOR_TYPE_NODE_GENERAL));
	EXPECT_EQ(RESULT_OK, eftApex.setScaleFactorIdentifier(4, 102));
	const int indexes[] = { 0, 1, 2, 3, 4 };
	// set parameter mappings for each function and term (two terms summed for first derivatives at apex)
	// the first two nodes as understood by the basis are collapsed into one
	// basis node 1 -> local node 1
	EXPECT_EQ(1, eftApex.getFunctionNumberOfTerms(1));
	EXPECT_EQ(RESULT_OK, eftApex.setTermNodeParameter(1, /*term*/1, /*localNode*/1, Node::VALUE_LABEL_VALUE, /*version*/1));
	// 0 terms = zero parameter for d/dxi1 basis
	EXPECT_EQ(RESULT_OK, eftApex.setFunctionNumberOfTerms(2, 0));
	// 2 terms for d/dxi2 via general linear map
	EXPECT_EQ(RESULT_OK, eftApex.setFunctionNumberOfTerms(3, 2));
	EXPECT_EQ(RESULT_OK, eftApex.setTermNodeParameter(3, /*term*/1, /*localNode*/1, Node::VALUE_LABEL_D_DS1, /*version*/1));
	EXPECT_EQ(RESULT_OK, eftApex.setTermScaling(3, /*term1*/1, /*indexesCount*/1, indexes + 1));
	EXPECT_EQ(RESULT_OK, eftApex.setTermNodeParameter(3, /*term*/2, /*localNode*/1, Node::VALUE_LABEL_D_DS2, /*version*/1));
	EXPECT_EQ(RESULT_OK, eftApex.setTermScaling(3, /*term1*/2, /*count*/1, indexes + 2));
	// 0 terms = zero parameter for cross derivative
	EXPECT_EQ(RESULT_OK, eftApex.setFunctionNumberOfTerms(4, 0));
	// basis node 2 -> local node 1
	EXPECT_EQ(1, eftApex.getFunctionNumberOfTerms(5));
	EXPECT_EQ(RESULT_OK, eftApex.setTermNodeParameter(5, /*term*/1, /*localNode*/1, Node::VALUE_LABEL_VALUE, /*version*/1));
	// 0 terms = zero parameter for d/dxi1 basis
	EXPECT_EQ(RESULT_OK, eftApex.setFunctionNumberOfTerms(6, 0));
	// 2 terms for d/dxi2 via general linear map
	EXPECT_EQ(RESULT_OK, eftApex.setFunctionNumberOfTerms(7, 2));
	EXPECT_EQ(RESULT_OK, eftApex.setTermNodeParameter(7, /*term*/1, /*localNode*/1, Node::VALUE_LABEL_D_DS1, /*version*/1));
	EXPECT_EQ(RESULT_OK, eftApex.setTermScaling(7, /*term1*/1, /*indexesCount*/1, indexes + 3));
	EXPECT_EQ(RESULT_OK, eftApex.setTermNodeParameter(7, /*term*/2, /*localNode*/1, Node::VALUE_LABEL_D_DS2, /*version*/1));
	EXPECT_EQ(RESULT_OK, eftApex.setTermScaling(7, /*term1*/2, /*count*/1, indexes + 4));
	// 0 terms = zero parameter for cross derivative
	EXPECT_EQ(RESULT_OK, eftApex.setFunctionNumberOfTerms(8, 0));
	// basis nodes 3, 4  -> local nodes 2, 3
	for (int n = 2; n < 4; ++n)
	{
		const int baseFunction = n*4;
		for (int d = 1; d <= 4; ++d)
		{
			// rely on derivatives being in fixed order:
			Node::ValueLabel valueLabel = static_cast<Node::ValueLabel>(Node::VALUE_LABEL_VALUE + d - 1);
			EXPECT_EQ(1, eftApex.getFunctionNumberOfTerms(baseFunction + d));
			EXPECT_EQ(RESULT_OK, eftApex.setTermNodeParameter(baseFunction + d, /*term*/1, /*localNode*/n,
				valueLabel, /*version*/1));
		}
	}

	Elementtemplate elementtemplate = mesh.createElementtemplate();
	EXPECT_TRUE(elementtemplate.isValid());
	EXPECT_EQ(RESULT_OK, elementtemplate.setElementShapeType(Element::SHAPE_TYPE_SQUARE));
	EXPECT_EQ(RESULT_OK, elementtemplate.defineField(coordinates, -1, eft));

	Elementtemplate elementtemplateApex = mesh.createElementtemplate();
	EXPECT_TRUE(elementtemplateApex.isValid());
	EXPECT_EQ(RESULT_OK, elementtemplateApex.setElementShapeType(Element::SHAPE_TYPE_SQUARE));
	EXPECT_EQ(RESULT_OK, elementtemplateApex.defineField(coordinates, -1, eftApex));

	// create model
	fm.beginChange();
	Fieldcache cache = fm.createFieldcache();
	EXPECT_TRUE(cache.isValid());

	// create nodes
	int nodeIdentifier = 1;
	const double radiansPerElementAround = 2.0*PI/elementsAroundCount;
	const double radiansPerElementUp = 0.5*PI/elementsUpCount;

	// create apex node
	const double zero3[3] = { 0.0, 0.0, 0.0 };
	const double dx_ds1_apex[3] = { radiansPerElementUp, 0.0, 0.0 };
	const double dx_ds2_apex[3] = { 0.0, radiansPerElementUp, 0.0 };
	Node node = nodes.createNode(nodeIdentifier++, nodetemplateApex);
	EXPECT_TRUE(node.isValid());
	EXPECT_EQ(RESULT_OK, cache.setNode(node));
	EXPECT_EQ(RESULT_OK, coordinates.setNodeParameters(cache, -1, Node::VALUE_LABEL_VALUE, 1, 3, zero3));
	EXPECT_EQ(RESULT_OK, coordinates.setNodeParameters(cache, -1, Node::VALUE_LABEL_D_DS1, 1, 3, dx_ds1_apex));
	EXPECT_EQ(RESULT_OK, coordinates.setNodeParameters(cache, -1, Node::VALUE_LABEL_D_DS2, 1, 3, dx_ds2_apex));

	// create hemisphere nodes
	for (int nu = 1; nu <= elementsUpCount; ++nu)
	{
		const double radiansUp = nu*radiansPerElementUp;
		const double cosRadiansUp = cos(radiansUp);
		const double sinRadiansUp = sin(radiansUp);
		for (int na = 0; na < elementsAroundCount; ++na)
		{
			const double radiansAround = na*radiansPerElementAround;
			const double cosRadiansAround = cos(radiansAround);
			const double sinRadiansAround = sin(radiansAround);
			const double x[3] = { cosRadiansAround*sinRadiansUp, sinRadiansAround*sinRadiansUp, 1.0 - cosRadiansUp };
			const double dx_ds1[3] = {
				-sinRadiansAround*sinRadiansUp*radiansPerElementAround,
				 cosRadiansAround*sinRadiansUp*radiansPerElementAround,
				 0.0 };
			const double dx_ds2[3] = {
				 cosRadiansAround*cosRadiansUp*radiansPerElementUp,
				 sinRadiansAround*cosRadiansUp*radiansPerElementUp,
				 sinRadiansUp*radiansPerElementUp };
			node = nodes.createNode(nodeIdentifier++, nodetemplate);
			EXPECT_TRUE(node.isValid());
			EXPECT_EQ(RESULT_OK, cache.setNode(node));
			EXPECT_EQ(RESULT_OK, coordinates.setNodeParameters(cache, -1, Node::VALUE_LABEL_VALUE, /*version*/1, 3, x));
			EXPECT_EQ(RESULT_OK, coordinates.setNodeParameters(cache, -1, Node::VALUE_LABEL_D_DS1, /*version*/1, 3, dx_ds1));
			EXPECT_EQ(RESULT_OK, coordinates.setNodeParameters(cache, -1, Node::VALUE_LABEL_D_DS2, /*version*/1, 3, dx_ds2));
			EXPECT_EQ(RESULT_OK, coordinates.setNodeParameters(cache, -1, Node::VALUE_LABEL_D2_DS1DS2, /*version*/1, 3, zero3));
 		}
	}

	// create tube nodes
	for (int nt = 1; nt <= elementsTubeCount; ++nt)
	{
		for (int na = 0; na < elementsAroundCount; ++na)
		{
			const double radiansAround = na*radiansPerElementAround;
			const double cosRadiansAround = cos(radiansAround);
			const double sinRadiansAround = sin(radiansAround);
			const double x[3] = { cosRadiansAround, sinRadiansAround, 1.0 + nt*radiansPerElementUp };
			const double dx_ds1[3] = {-sinRadiansAround*radiansPerElementAround, cosRadiansAround*radiansPerElementAround, 0.0 };
			const double dx_ds2[3] = { 0.0, 0.0, radiansPerElementUp };
			node = nodes.createNode(nodeIdentifier++, nodetemplate);
			EXPECT_TRUE(node.isValid());
			EXPECT_EQ(RESULT_OK, cache.setNode(node));
			EXPECT_EQ(RESULT_OK, coordinates.setNodeParameters(cache, -1, Node::VALUE_LABEL_VALUE, /*version*/1, 3, x));
			EXPECT_EQ(RESULT_OK, coordinates.setNodeParameters(cache, -1, Node::VALUE_LABEL_D_DS1, /*version*/1, 3, dx_ds1));
			EXPECT_EQ(RESULT_OK, coordinates.setNodeParameters(cache, -1, Node::VALUE_LABEL_D_DS2, /*version*/1, 3, dx_ds2));
			EXPECT_EQ(RESULT_OK, coordinates.setNodeParameters(cache, -1, Node::VALUE_LABEL_D2_DS1DS2, /*version*/1, 3, zero3));
		}
	}

	// create elements
	int elementIdentifier = 1;

	// create first row of elements at apex
	// note scale factor identifiers follow convention of offseting by 100 for each 'version' around apex
	for (int na = 0; na < elementsAroundCount; ++na)
	{
		const int va = na;
		const int vb = ((na + 1) % elementsAroundCount);
		eftApex.setScaleFactorIdentifier(1, va*100 + 1);
		eftApex.setScaleFactorIdentifier(2, va*100 + 2);
		eftApex.setScaleFactorIdentifier(3, vb*100 + 1);
		eftApex.setScaleFactorIdentifier(4, vb*100 + 2);
		// redefine field in template for changes to eftApex:
		EXPECT_EQ(RESULT_OK, elementtemplateApex.defineField(coordinates, -1, eftApex));
		Element element = mesh.createElement(elementIdentifier++, elementtemplateApex);
		const int nodeIdentifiers[3] = { 1, na + 2, ((na + 1) < elementsAroundCount) ? na + 3 : 2 };
		EXPECT_EQ(RESULT_OK, result = element.setNodesByIdentifier(eftApex, 3, nodeIdentifiers));
		// set general linear map coefficients
		const double radiansAround = na*radiansPerElementAround;
		const double cosRadiansAround = cos(radiansAround);
		const double sinRadiansAround = sin(radiansAround);
		// try to avoid numerical differences between first and last coefficients
		const double cosRadiansAroundNext = ((na + 1) < elementsAroundCount) ? cos(radiansAround + radiansPerElementAround) : cos(0);
		const double sinRadiansAroundNext = ((na + 1) < elementsAroundCount) ? sin(radiansAround + radiansPerElementAround) : sin(0);
		const double scalefactors[4] = { cosRadiansAround, sinRadiansAround, cosRadiansAroundNext, sinRadiansAroundNext };
		EXPECT_EQ(RESULT_OK, result = element.setScaleFactors(eftApex, 4, scalefactors));
	}

	// create remaining rows on hemisphere and tube
	const int elementsRemainingRowsCount = elementsUpCount + elementsTubeCount - 1;
	for (int nr = 0; nr < elementsRemainingRowsCount; ++nr)
	{
		for (int na = 0; na < elementsAroundCount; ++na)
		{
			const int baseNodeIdentifier = 2 + elementsAroundCount*nr + na;
			const int nodeIdentifierNextOffset = ((na + 1) < elementsAroundCount) ? 1 : 1 - elementsAroundCount;
			Element element = mesh.createElement(elementIdentifier++, elementtemplate);
			const int nodeIdentifiers[4] = { baseNodeIdentifier, baseNodeIdentifier + nodeIdentifierNextOffset,
				baseNodeIdentifier + elementsAroundCount, baseNodeIdentifier + elementsAroundCount + nodeIdentifierNextOffset };
			EXPECT_EQ(RESULT_OK, result = element.setNodesByIdentifier(eft, 4, nodeIdentifiers));
		}
	}
	fm.endChange();
}

void checkHemisphereTube2d(Fieldmodule& fm, int elementsAroundCount, int elementsUpCount, int elementsTubeCount)
{
	int result;
	Field coordinates = fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	EXPECT_EQ(3, coordinates.getNumberOfComponents());
	EXPECT_TRUE(coordinates.isTypeCoordinate());

	EXPECT_EQ(RESULT_OK, fm.defineAllFaces());
	Mesh mesh2d = fm.findMeshByDimension(2);
	EXPECT_EQ(elementsAroundCount*(elementsUpCount + elementsTubeCount), mesh2d.getSize());
	Mesh mesh1d = fm.findMeshByDimension(1);
	EXPECT_EQ(2*elementsAroundCount*(elementsUpCount + elementsTubeCount), mesh1d.getSize());
	Nodeset nodes = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_EQ(elementsAroundCount*(elementsUpCount + elementsTubeCount) + 1, nodes.getSize());

	const double valueOne = 1.0;
	Field one = fm.createFieldConstant(1, &valueOne);
	FieldMeshIntegral area = fm.createFieldMeshIntegral(one, coordinates, mesh2d);
	EXPECT_TRUE(area.isValid());
	const int numGaussPoints = 4;
	EXPECT_EQ(RESULT_OK, result = area.setNumbersOfPoints(1, &numGaussPoints));

	// Surface area of a sphere is 4*PI*r^2, hemisphere is half. Add tube
	const double radiansPerElementUp = 0.5*PI/elementsUpCount;
	const double exactArea = 2*PI*(1.0 + radiansPerElementUp);

	Fieldcache cache = fm.createFieldcache();
	double areaOut;
	EXPECT_EQ(RESULT_OK, result = area.evaluateReal(cache, 1, &areaOut));
	EXPECT_NEAR(exactArea, areaOut, 0.03);
	// test approximated value for one refinement
	if ((elementsAroundCount == 6) && (elementsUpCount == 3) && (elementsTubeCount == 1))
	{
		EXPECT_NEAR(9.5434143995913274, areaOut, 1.0E-5);
	}

	// check element field template definition
	Element element1 = mesh2d.findElementByIdentifier(1);
	EXPECT_TRUE(element1.isValid());
	Elementfieldtemplate eft = element1.getElementfieldtemplate(coordinates, /*all_component*/-1);
	EXPECT_TRUE(eft.isValid());
	Elementbasis elementbasis = eft.getElementbasis();
	EXPECT_TRUE(elementbasis.isValid());
	EXPECT_EQ(Elementbasis::FUNCTION_TYPE_CUBIC_HERMITE, elementbasis.getFunctionType(/*all_xi*/-1));
	EXPECT_EQ(3, eft.getNumberOfLocalNodes());
	EXPECT_EQ(4, eft.getNumberOfLocalScaleFactors());
	for (int i = 1; i <= 4; ++i)
	{
		EXPECT_EQ(Elementfieldtemplate::SCALE_FACTOR_TYPE_NODE_GENERAL, eft.getScaleFactorType(i));
		EXPECT_EQ((i < 3) ? i : 98 + i, eft.getScaleFactorIdentifier(i));
	}

	EXPECT_EQ(16, eft.getNumberOfFunctions());
	for (int f = 1; f <= 16; ++f)
	{
		const int termCount = eft.getFunctionNumberOfTerms(f);
		if ((f == 2) || (f == 4) || (f == 6) || (f == 8))
		{
			EXPECT_EQ(0, termCount);
		}
		else if ((f == 1) || (f == 5) || (f > 8))
		{
			EXPECT_EQ(1, termCount);
			const int expectedLocalNodeIndex = (f <= 8) ? 1 : ((f <= 12) ? 2 : 3);
			EXPECT_EQ(expectedLocalNodeIndex, eft.getTermLocalNodeIndex(f, 1));
			const int derivativeCase = (f - 1) % 4;
			Node::ValueLabel expectedValueLabel =
				(derivativeCase == 0) ? Node::VALUE_LABEL_VALUE :
				(derivativeCase == 1) ? Node::VALUE_LABEL_D_DS1 :
				(derivativeCase == 2) ? Node::VALUE_LABEL_D_DS2 : Node::VALUE_LABEL_D2_DS1DS2;
			EXPECT_EQ(expectedValueLabel, eft.getTermNodeValueLabel(f, 1));
			EXPECT_EQ(1, eft.getTermNodeVersion(f, 1));
		}
		else // ((f == 3) || (f == 7))
		{
			EXPECT_EQ(2, termCount);
			EXPECT_EQ(1, eft.getTermLocalNodeIndex(f, 1));
			EXPECT_EQ(1, eft.getTermLocalNodeIndex(f, 2));
			EXPECT_EQ(Node::VALUE_LABEL_D_DS1, eft.getTermNodeValueLabel(f, 1));
			EXPECT_EQ(Node::VALUE_LABEL_D_DS2, eft.getTermNodeValueLabel(f, 2));
			EXPECT_EQ(1, eft.getTermNodeVersion(f, 1));
			EXPECT_EQ(1, eft.getTermNodeVersion(f, 2));
			int scaleFactorIndex1, scaleFactorIndex2;
			EXPECT_EQ(RESULT_OK, eft.getTermScaling(f, 1, 1, &scaleFactorIndex1));
			EXPECT_EQ(RESULT_OK, eft.getTermScaling(f, 2, 1, &scaleFactorIndex2));
			EXPECT_EQ((f == 3) ? 1 : 3, scaleFactorIndex1);
			EXPECT_EQ((f == 3) ? 2 : 4, scaleFactorIndex2);
		}
	}

	// test local nodes API for one element
	Node node1 = nodes.findNodeByIdentifier(1);
	EXPECT_TRUE(node1.isValid());
	Node node2 = nodes.findNodeByIdentifier(2);
	EXPECT_TRUE(node2.isValid());
	Node node3 = nodes.findNodeByIdentifier(3);
	EXPECT_TRUE(node3.isValid());
	Node node4 = nodes.findNodeByIdentifier(4);
	EXPECT_TRUE(node4.isValid());
	Node elementNode1 = element1.getNode(eft, 1);
	EXPECT_EQ(node1, elementNode1);
	Node elementNode2 = element1.getNode(eft, 2);
	EXPECT_EQ(node2, elementNode2);
	Node elementNode3 = element1.getNode(eft, 3);
	EXPECT_EQ(node3, elementNode3);
	// test setting node and restore it afterwards
	EXPECT_EQ(RESULT_OK, element1.setNode(eft, 3, node4));
	elementNode3 = element1.getNode(eft, 3);
	EXPECT_EQ(node4, elementNode3);
	EXPECT_EQ(RESULT_OK, element1.setNode(eft, 3, node3));
	elementNode3 = element1.getNode(eft, 3);
	EXPECT_EQ(node3, elementNode3);

	// test scale factors for one element (only valid for one refinement)
	if ((elementsAroundCount == 6) && (elementsUpCount == 3) && (elementsTubeCount == 1))
	{
		double scaleFactorsOrig[4];
		const double scaleFactorsExpected[4] = { 1.0, 0.0, 0.5, 0.8660254037844386 };
		EXPECT_EQ(RESULT_OK, element1.getScaleFactors(eft, 4, scaleFactorsOrig));
		for (int i = 0; i < 4; ++i)
			EXPECT_NEAR(scaleFactorsExpected[i], scaleFactorsOrig[i], 1.0E-13);
		double sfDummy;
		EXPECT_EQ(RESULT_ERROR_ARGUMENT, element1.getScaleFactor(eft, 0, &sfDummy));
		for (int i = 1; i <= 4; ++i)
		{
			double sfOut;
			EXPECT_EQ(RESULT_OK, element1.getScaleFactor(eft, i, &sfOut));
			EXPECT_DOUBLE_EQ(scaleFactorsOrig[i - 1], sfOut);
			const double sf = i*1.25;
			EXPECT_EQ(RESULT_OK, element1.setScaleFactor(eft, i, sf));
			EXPECT_EQ(RESULT_OK, element1.getScaleFactor(eft, i, &sfOut));
			EXPECT_DOUBLE_EQ(sf, sfOut);
		}
		EXPECT_EQ(RESULT_ERROR_ARGUMENT, element1.getScaleFactor(eft, 5, &sfDummy));
		EXPECT_EQ(RESULT_OK, element1.setScaleFactors(eft, 4, scaleFactorsOrig));
	}
	else
	{
		EXPECT_TRUE(false); // to make sure we are testing the above
	}
}

}

/* Test creating a hemisphere with general linear mapped apex with connected tube */
TEST(FieldIO, generateHemisphereTube2d)
{
	ZincTestSetupCpp zinc;
	int result;

	const int elementsAroundCount = 6;
	const int elementsUpCount = 3;
	const int elementsTubeCount = 1;
	createHemisphereTube2d(zinc.fm, elementsAroundCount, elementsUpCount, elementsTubeCount);
	checkHemisphereTube2d(zinc.fm, elementsAroundCount, elementsUpCount, elementsTubeCount);

	// test writing and re-reading in EX2 format
	EXPECT_EQ(RESULT_OK, result = zinc.root_region.writeFile(FIELDML_OUTPUT_FOLDER "/hemisphere_tube.ex2"));
	Region testRegion1 = zinc.root_region.createChild("test1");
	EXPECT_EQ(RESULT_OK, result = testRegion1.readFile(FIELDML_OUTPUT_FOLDER "/hemisphere_tube.ex2"));
	Fieldmodule testFm1 = testRegion1.getFieldmodule();
	checkHemisphereTube2d(testFm1, elementsAroundCount, elementsUpCount, elementsTubeCount);
}

namespace {

void createMultipleSharedScaleFactorsModel(Fieldmodule& fm,
	const int elementCountX, const int elementCountY, const double sizeX, const double sizeY)
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
	EXPECT_EQ(RESULT_OK, nodetemplate.setValueNumberOfVersions(coordinates, -1, Node::VALUE_LABEL_VALUE, 1));
	EXPECT_EQ(RESULT_OK, nodetemplate.setValueNumberOfVersions(coordinates, -1, Node::VALUE_LABEL_D_DS1, 1));
	EXPECT_EQ(RESULT_OK, nodetemplate.setValueNumberOfVersions(coordinates, -1, Node::VALUE_LABEL_D_DS2, 1));
	EXPECT_EQ(RESULT_OK, nodetemplate.setValueNumberOfVersions(coordinates, -1, Node::VALUE_LABEL_D2_DS1DS2, 1));

	Elementbasis bicubicHermiteBasis = fm.createElementbasis(2, Elementbasis::FUNCTION_TYPE_CUBIC_HERMITE);
	EXPECT_TRUE(bicubicHermiteBasis.isValid());

	Mesh mesh = fm.findMeshByDimension(2);
	EXPECT_TRUE(mesh.isValid());

	// main element field template for regular elements away from apex is the default bicubic Hermite mapping
	Elementfieldtemplate eft = mesh.createElementfieldtemplate(bicubicHermiteBasis);
	EXPECT_TRUE(eft.isValid());
	EXPECT_EQ(Elementfieldtemplate::PARAMETER_MAPPING_MODE_NODE, eft.getParameterMappingMode());
	// element has 2 scale factors per node for the first derivatives, and in product for the cross derivative
	EXPECT_EQ(RESULT_OK, eft.setNumberOfLocalScaleFactors(8));
	EXPECT_EQ(8, eft.getNumberOfLocalScaleFactors());
	for (int s = 0; s < 8; ++s)
	{
		EXPECT_EQ(RESULT_OK, eft.setScaleFactorType(s + 1, Elementfieldtemplate::SCALE_FACTOR_TYPE_NODE_PATCH));
		EXPECT_EQ(RESULT_OK, eft.setScaleFactorIdentifier(s + 1, s % 2 + 1));
	}
	EXPECT_EQ(16, eft.getNumberOfFunctions());
	for (int n = 0; n < 4; ++n)
	{
		const int scaleFactorIndexes[2] = { n*2 + 1, n*2 + 2 };
		EXPECT_EQ(RESULT_OK, eft.setTermScaling(n*4 + 2, /*term*/1, 1, scaleFactorIndexes));
		EXPECT_EQ(RESULT_OK, eft.setTermScaling(n*4 + 3, /*term*/1, 1, scaleFactorIndexes + 1));
		EXPECT_EQ(RESULT_OK, eft.setTermScaling(n*4 + 4, /*term*/1, 2, scaleFactorIndexes));
	}
	EXPECT_TRUE(eft.validate());

	Elementtemplate elementtemplate = mesh.createElementtemplate();
	EXPECT_TRUE(elementtemplate.isValid());
	EXPECT_EQ(RESULT_OK, elementtemplate.setElementShapeType(Element::SHAPE_TYPE_SQUARE));
	EXPECT_EQ(RESULT_OK, elementtemplate.defineField(coordinates, -1, eft));

	// create model
	const double spacingX = sizeX / elementCountX;
	const double spacingY = sizeY / elementCountY;

	fm.beginChange();
	Fieldcache cache = fm.createFieldcache();
	EXPECT_TRUE(cache.isValid());

	// create nodes
	int nodeIdentifier = 1;
	const double dx_ds1[3] = { 1.0, 0.0, 0.0 };
	const double dx_ds2[3] = { 0.0, 1.0, 0.0 };
	const double zero3[3] = { 0.0, 0.0, 0.0 };
	for (int ny = 0; ny <= elementCountY; ++ny)
	{
		for (int nx = 0; nx <= elementCountX; ++nx)
		{
			Node node = nodes.createNode(nodeIdentifier++, nodetemplate);
			EXPECT_TRUE(node.isValid());
			EXPECT_EQ(RESULT_OK, cache.setNode(node));
			const double x[3] = { nx*spacingX, ny*spacingY, 0.0 };
			EXPECT_EQ(RESULT_OK, coordinates.setNodeParameters(cache, -1, Node::VALUE_LABEL_VALUE, /*version*/1, 3, x));
			EXPECT_EQ(RESULT_OK, coordinates.setNodeParameters(cache, -1, Node::VALUE_LABEL_D_DS1, /*version*/1, 3, dx_ds1));
			EXPECT_EQ(RESULT_OK, coordinates.setNodeParameters(cache, -1, Node::VALUE_LABEL_D_DS2, /*version*/1, 3, dx_ds2));
			EXPECT_EQ(RESULT_OK, coordinates.setNodeParameters(cache, -1, Node::VALUE_LABEL_D2_DS1DS2, /*version*/1, 3, zero3));
		}
	}

	// create elements
	int elementIdentifier = 1;
	const double scaleFactors[8] = { spacingX, spacingY, spacingX, spacingY, spacingX, spacingY, spacingX, spacingY };
	for (int ey = 0; ey < elementCountY; ++ey)
	{
		for (int ex = 0; ex < elementCountX; ++ex)
		{
			Element element = mesh.createElement(elementIdentifier++, elementtemplate);
			EXPECT_TRUE(element.isValid());
			const int ei = ey*(elementCountX + 1) + ex + 1;
			const int nodeIdentifiers[4] = { ei, ei + 1, ei + elementCountX + 1, ei + elementCountX + 2 };
			EXPECT_EQ(RESULT_OK, element.setNodesByIdentifier(eft, 4, nodeIdentifiers));
			EXPECT_EQ(RESULT_OK, element.setScaleFactors(eft, 8, scaleFactors));
		}
	}
	fm.endChange();
}

void checkMultipleSharedScaleFactorsModel(Fieldmodule& fm,
	const int elementCountX, const int elementCountY, const double sizeX, const double sizeY)
{
	const double spacingX = sizeX / elementCountX;
	const double spacingY = sizeY / elementCountY;
	int result;
	FieldFiniteElement coordinates = fm.findFieldByName("coordinates").castFiniteElement();
	EXPECT_TRUE(coordinates.isValid());
	EXPECT_EQ(3, coordinates.getNumberOfComponents());
	EXPECT_TRUE(coordinates.isTypeCoordinate());

	EXPECT_EQ(RESULT_OK, fm.defineAllFaces());
	Mesh mesh2d = fm.findMeshByDimension(2);
	EXPECT_EQ(elementCountX*elementCountY, mesh2d.getSize());
	Mesh mesh1d = fm.findMeshByDimension(1);
	EXPECT_EQ((elementCountX + 1)*elementCountY + elementCountX*(elementCountY + 1), mesh1d.getSize());
	Nodeset nodes = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_EQ((elementCountX + 1)*(elementCountY + 1), nodes.getSize());

	const double valueOne = 1.0;
	Field one = fm.createFieldConstant(1, &valueOne);
	FieldMeshIntegral area = fm.createFieldMeshIntegral(one, coordinates, mesh2d);
	EXPECT_TRUE(area.isValid());
	const int numGaussPoints = 4;
	EXPECT_EQ(RESULT_OK, result = area.setNumbersOfPoints(1, &numGaussPoints));

	Fieldcache cache = fm.createFieldcache();
	double areaOut;
	EXPECT_EQ(RESULT_OK, result = area.evaluateReal(cache, 1, &areaOut));
	const double areaTol = 1.0E-5;
	EXPECT_NEAR(sizeX*sizeY, areaOut, areaTol);

	Element element4 = mesh2d.findElementByIdentifier(4);
	EXPECT_TRUE(element4.isValid());

	// check element field template definition
	Elementfieldtemplate eft = element4.getElementfieldtemplate(coordinates, -1);
	EXPECT_TRUE(eft.isValid());
	Elementbasis elementbasis = eft.getElementbasis();
	EXPECT_TRUE(elementbasis.isValid());
	EXPECT_EQ(Elementbasis::FUNCTION_TYPE_CUBIC_HERMITE, elementbasis.getFunctionType(/*all_xi*/-1));
	EXPECT_EQ(4, eft.getNumberOfLocalNodes());
	EXPECT_EQ(8, eft.getNumberOfLocalScaleFactors());
	for (int s = 0; s < 8; ++s)
	{
		EXPECT_EQ(Elementfieldtemplate::SCALE_FACTOR_TYPE_NODE_PATCH, eft.getScaleFactorType(s + 1));
		EXPECT_EQ(s % 2 + 1, eft.getScaleFactorIdentifier(s + 1));
	}
	for (int n = 0; n < 4; ++n)
	{
		const int expectedScaleFactorIndexes[2] = { n*2 + 1, n*2 + 2 };
		int scaleFactorIndexOut, scaleFactorIndexesOut[2];
		EXPECT_EQ(0, eft.getTermScaling(n*4 + 1, /*term*/1, 0, 0));
		EXPECT_EQ(1, eft.getTermScaling(n*4 + 2, /*term*/1, 1, &scaleFactorIndexOut));
		EXPECT_EQ(expectedScaleFactorIndexes[0], scaleFactorIndexOut);
		EXPECT_EQ(1, eft.getTermScaling(n*4 + 3, /*term*/1, 1, &scaleFactorIndexOut));
		EXPECT_EQ(expectedScaleFactorIndexes[1], scaleFactorIndexOut);
		EXPECT_EQ(2, eft.getTermScaling(n*4 + 4, /*term*/1, 2, scaleFactorIndexesOut));
		EXPECT_EQ(expectedScaleFactorIndexes[0], scaleFactorIndexesOut[0]);
		EXPECT_EQ(expectedScaleFactorIndexes[1], scaleFactorIndexesOut[1]);
	}

	// check all scale factors match originals
	const int elementCount = elementCountX*elementCountY;
	double scaleFactorsOut[8];
	const double expectedScaleFactors[8] = { spacingX, spacingY, spacingX, spacingY, spacingX, spacingY, spacingX, spacingY };
	for (int e = 1; e <= elementCount; ++e)
	{
		Element element = mesh2d.findElementByIdentifier(e);
		EXPECT_TRUE(element.isValid());
		EXPECT_EQ(RESULT_OK, element.getScaleFactors(eft, 8, scaleFactorsOut));
		for (int i = 0; i < 8; ++i)
		{
			EXPECT_DOUBLE_EQ(expectedScaleFactors[i], scaleFactorsOut[i]);
		}
	}

	// test editing for one refinement (assume sizeX = 10, sizeY = 5)
	if ((elementCountX == 2) && (elementCountY == 2))
	{
		// test swapping nodes maintains node-based scalefactors
		const int expectedNodeIdentifiers[4] = { 5, 6, 8, 9 };
		const int changeNodeIdentifiers[4] = { 5, 9, 8, 6 };
		for (int n = 0; n < 4; ++n)
		{
			Node node = element4.getNode(eft, n + 1);
			EXPECT_TRUE(node.isValid());
			EXPECT_EQ(expectedNodeIdentifiers[n], node.getIdentifier());
		}
		EXPECT_EQ(RESULT_OK, element4.setNodesByIdentifier(eft, 4, changeNodeIdentifiers));
		// check scale factors are the same i.e. not lost from node 9
		EXPECT_EQ(RESULT_OK, element4.getScaleFactors(eft, 8, scaleFactorsOut));
		for (int i = 0; i < 8; ++i)
		{
			EXPECT_DOUBLE_EQ(expectedScaleFactors[i], scaleFactorsOut[i]);
		}
		// twist in element 4 reduces its area
		double newAreaOut;
		EXPECT_EQ(RESULT_OK, result = area.evaluateReal(cache, 1, &newAreaOut));
		EXPECT_NEAR(47.278608941112310, newAreaOut, areaTol);

		// test temporarily removing node 9 loses its node-based scale factors, but recovers those of node 6 when reinstated
		Node node6 = nodes.findNodeByIdentifier(6);
		EXPECT_TRUE(node6.isValid());
		Node node9 = nodes.findNodeByIdentifier(9);
		EXPECT_TRUE(node9.isValid());
		EXPECT_EQ(RESULT_OK, element4.setNode(eft, 2, node6));
		// to check this, set the scale factors at local node 2, now that it is global node 6
		EXPECT_EQ(RESULT_OK, element4.setScaleFactor(eft, 3, 1.5*spacingX));
		EXPECT_EQ(RESULT_OK, element4.setScaleFactor(eft, 4, 1.5*spacingY));
		// reducing side of element 4 to a point reduces its area further
		EXPECT_EQ(RESULT_OK, result = area.evaluateReal(cache, 1, &newAreaOut));
		EXPECT_NEAR(45.534285867263037, newAreaOut, areaTol);
		EXPECT_EQ(RESULT_OK, element4.setNode(eft, 4, node9));
		EXPECT_EQ(RESULT_OK, element4.getScaleFactors(eft, 8, scaleFactorsOut));
		for (int i = 0; i < 8; ++i)
		{
			const double scaling = (((i / 2) % 2) == 0) ? 1.0 : 1.5;
			EXPECT_DOUBLE_EQ(scaling*expectedScaleFactors[i], scaleFactorsOut[i]);
		}
		// restoring node 9, but with different scale factors actually gives the original area
		EXPECT_EQ(RESULT_OK, result = area.evaluateReal(cache, 1, &newAreaOut));
		EXPECT_NEAR(sizeX*sizeY, newAreaOut, areaTol);
		// restore correct scale factors
		EXPECT_EQ(RESULT_OK, element4.setScaleFactors(eft, 8, expectedScaleFactors));
		EXPECT_EQ(RESULT_OK, result = area.evaluateReal(cache, 1, &newAreaOut));
		EXPECT_NEAR(sizeX*sizeY, newAreaOut, areaTol);

		// test changes to common node scale factor affects other elements
		Node node8 = nodes.findNodeByIdentifier(8);
		EXPECT_TRUE(node8.isValid());
		cache.setNode(node8);
		const double newY = sizeY*1.25;
		EXPECT_EQ(RESULT_OK, coordinates.setNodeParameters(cache, /*componentNumber*/2, Node::VALUE_LABEL_VALUE, 1, 1, &newY));
		EXPECT_EQ(RESULT_OK, element4.setScaleFactor(eft, 2, spacingY*1.05));
		EXPECT_EQ(RESULT_OK, element4.setScaleFactor(eft, 5, spacingX*1.10));
		EXPECT_EQ(RESULT_OK, element4.setScaleFactor(eft, 6, spacingY*1.5));
		Element element3 = mesh2d.findElementByIdentifier(3);
		EXPECT_TRUE(element3.isValid());
		EXPECT_EQ(RESULT_OK, element3.getScaleFactors(eft, 8, scaleFactorsOut));
		EXPECT_DOUBLE_EQ(spacingX, scaleFactorsOut[0]);
		EXPECT_DOUBLE_EQ(spacingY, scaleFactorsOut[1]);
		EXPECT_DOUBLE_EQ(spacingX, scaleFactorsOut[2]);
		EXPECT_DOUBLE_EQ(spacingY*1.05, scaleFactorsOut[3]);
		EXPECT_DOUBLE_EQ(spacingX, scaleFactorsOut[4]);
		EXPECT_DOUBLE_EQ(spacingY, scaleFactorsOut[5]);
		EXPECT_DOUBLE_EQ(spacingX*1.10, scaleFactorsOut[6]);
		EXPECT_DOUBLE_EQ(spacingY*1.5, scaleFactorsOut[7]);
		EXPECT_EQ(RESULT_OK, result = area.evaluateReal(cache, 1, &newAreaOut));
		EXPECT_NEAR(56.375, newAreaOut, areaTol);

		// restore original coordinates and scale factors
		EXPECT_EQ(RESULT_OK, coordinates.setNodeParameters(cache, /*componentNumber*/2, Node::VALUE_LABEL_VALUE, 1, 1, &sizeY));
		EXPECT_EQ(RESULT_OK, element4.setScaleFactors(eft, 8, expectedScaleFactors));

		// set cross derivative in centre and check it is scaled
		const double cross = 2.0;
		Node node5 = nodes.findNodeByIdentifier(5);
		EXPECT_TRUE(node5.isValid());
		cache.setNode(node5);
		EXPECT_EQ(RESULT_OK, coordinates.setNodeParameters(cache, /*componentNumber*/3, Node::VALUE_LABEL_D2_DS1DS2, 1, 1, &cross));
		EXPECT_EQ(RESULT_OK, result = area.evaluateReal(cache, 1, &newAreaOut));
		EXPECT_NEAR(53.530023195850092, newAreaOut, areaTol);
		// set zero node scale factors in centre and check original flat area returned
		EXPECT_EQ(RESULT_OK, element4.setScaleFactor(eft, 1, 0.0));
		EXPECT_EQ(RESULT_OK, element4.setScaleFactor(eft, 2, 0.0));
		EXPECT_EQ(RESULT_OK, result = area.evaluateReal(cache, 1, &newAreaOut));
		EXPECT_NEAR(sizeX*sizeY, newAreaOut, areaTol);

		// restore original coordinates and scale factors
		const double zero = 0.0;
		EXPECT_EQ(RESULT_OK, coordinates.setNodeParameters(cache, /*componentNumber*/3, Node::VALUE_LABEL_D2_DS1DS2, 1, 1, &zero));
		EXPECT_EQ(RESULT_OK, element4.setScaleFactors(eft, 8, expectedScaleFactors));
	}
}

}

// Test a model scaling cross derivatives by explicit product of shared node-based scale factors.
// Also test discovering and keeping them when element nodes changed.
TEST(FieldIO, multipleSharedScaleFactors)
{
	ZincTestSetupCpp zinc;
	int result;

	const int elementCountX = 2;
	const int elementCountY = 2;
	const double sizeX = 10.0;
	const double sizeY = 5.0;
	createMultipleSharedScaleFactorsModel(zinc.fm, elementCountX, elementCountY, sizeX, sizeY);
	checkMultipleSharedScaleFactorsModel(zinc.fm, elementCountX, elementCountY, sizeX, sizeY);

	// test writing and re-reading in EX2 format
	EXPECT_EQ(RESULT_OK, result = zinc.root_region.writeFile(FIELDML_OUTPUT_FOLDER "/multiple_scalefactors.ex2"));
	Region testRegion1 = zinc.root_region.createChild("test1");
	EXPECT_EQ(RESULT_OK, result = testRegion1.readFile(FIELDML_OUTPUT_FOLDER "/multiple_scalefactors.ex2"));
	Fieldmodule testFm1 = testRegion1.getFieldmodule();
	checkMultipleSharedScaleFactorsModel(testFm1, elementCountX, elementCountY, sizeX, sizeY);
}
