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
#include <opencmiss/zinc/fieldconstant.hpp>
#include <opencmiss/zinc/fieldparameters.hpp>
#include <opencmiss/zinc/fieldcache.hpp>
#include <opencmiss/zinc/fieldfiniteelement.hpp>

#include "utilities/zinctestsetupcpp.hpp"
#include "test_resources.h"

TEST(ZincFieldparameters, validAPI)
{
	ZincTestSetupCpp zinc;

	EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE)));

	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	const int componentCount = coordinates.getNumberOfComponents();
	EXPECT_EQ(3, componentCount);

	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	Element element = mesh3d.findElementByIdentifier(1);
	EXPECT_TRUE(element.isValid());

	Fieldparameters fieldparameters = coordinates.getFieldparameters();
	EXPECT_TRUE(fieldparameters.isValid());
	Field fieldOut = fieldparameters.getField();
	EXPECT_TRUE(fieldOut.isValid());
	EXPECT_EQ(coordinates, fieldOut);

	EXPECT_EQ(24, fieldparameters.getNumberOfElementParameters(element));
	EXPECT_EQ(24, fieldparameters.getNumberOfParameters());

	Differentialoperator parameterDerivative1 = fieldparameters.getDerivativeOperator(/*order*/1);
	EXPECT_TRUE(parameterDerivative1.isValid());
	Differentialoperator parameterDerivative2 = fieldparameters.getDerivativeOperator(/*order*/2);
	EXPECT_TRUE(parameterDerivative2.isValid());

	Fieldcache fieldcache = zinc.fm.createFieldcache();
	EXPECT_TRUE(fieldcache.isValid());

	double outDerivatives1[72];
	double outDerivatives2[1728];

	const double xi[4][3] =
	{
		{ 0.2, 0.5, 0.7 },
		{ 0.0, 0.9, 0.2 },
		{ 0.7, 0.2, 0.7 },
		{ 0.9, 0.9, 1.0 }
	};
	const double TOL = 1.0E-12;
	for (int i = 0; i < 4; ++i)
	{
		EXPECT_EQ(RESULT_OK, fieldcache.setMeshLocation(element, 3, xi[i]));
		EXPECT_EQ(RESULT_OK, coordinates.evaluateDerivative(parameterDerivative1, fieldcache, 72, outDerivatives1));
		// test that first derivatives of components w.r.t. their parameters equal the basis functions
		// but are zero w.r.t. other components' parameters
		const double linearBasis[8] =
		{
			(1.0 - xi[i][0])*(1.0 - xi[i][1])*(1.0 - xi[i][2]),
				   xi[i][0] *(1.0 - xi[i][1])*(1.0 - xi[i][2]),
			(1.0 - xi[i][0])*       xi[i][1] *(1.0 - xi[i][2]),
				   xi[i][0] *       xi[i][1] *(1.0 - xi[i][2]),
			(1.0 - xi[i][0])*(1.0 - xi[i][1])*       xi[i][2],
				   xi[i][0] *(1.0 - xi[i][1])*       xi[i][2],
			(1.0 - xi[i][0])*       xi[i][1] *       xi[i][2],
				   xi[i][0] *       xi[i][1] *       xi[i][2]
		};
		int v = 0;
		for (int c = 0; c < componentCount; ++c)
		{
			for (int d = 0; d < componentCount; ++d)
			{
				if (d == c)
				{
					for (int e = 0; e < 8; ++e)
						EXPECT_NEAR(linearBasis[e], outDerivatives1[v++], TOL);
				}
				else
				{
					for (int e = 0; e < 8; ++e)
						EXPECT_NEAR(0.0, outDerivatives1[v++], TOL);
				}
			}
		}
		EXPECT_EQ(RESULT_OK, coordinates.evaluateDerivative(parameterDerivative2, fieldcache, 1728, outDerivatives2));
		// test that second derivatives w.r.t. parameters are all zero
		for (int j = 0; j < 1728; ++j)
			EXPECT_DOUBLE_EQ(0.0, outDerivatives2[j]);
	}
}

TEST(ZincFieldparameters, invalidAPI)
{
	ZincTestSetupCpp zinc;

	FieldFiniteElement fieldFiniteElement = zinc.fm.createFieldFiniteElement(1);
	EXPECT_TRUE(fieldFiniteElement.isValid());
	Element noElement;
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	Elementtemplate elementtemplate = mesh3d.createElementtemplate();
	EXPECT_EQ(RESULT_OK, elementtemplate.setElementShapeType(Element::SHAPE_TYPE_CUBE));
	Element element = mesh3d.createElement(1, elementtemplate);
	EXPECT_TRUE(element.isValid());

	Fieldparameters fieldparameters = fieldFiniteElement.getFieldparameters();
	EXPECT_TRUE(fieldparameters.isValid());
	Field noField;
	EXPECT_FALSE(noField.isValid());
	Fieldparameters noFieldparameters = noField.getFieldparameters();
	EXPECT_FALSE(noFieldparameters.isValid());
	Field fieldOut = noFieldparameters.getField();
	EXPECT_FALSE(fieldOut.isValid());

	EXPECT_EQ(-1, fieldparameters.getNumberOfElementParameters(noElement));
	EXPECT_EQ(-1, noFieldparameters.getNumberOfElementParameters(element));
	EXPECT_EQ(-1, noFieldparameters.getNumberOfElementParameters(noElement));
	EXPECT_EQ(-1, noFieldparameters.getNumberOfParameters());

	Differentialoperator parameterDerivative1 = fieldparameters.getDerivativeOperator(/*order*/0);
	EXPECT_FALSE(parameterDerivative1.isValid());
	Differentialoperator parameterDerivative2 = fieldparameters.getDerivativeOperator(/*order*/3);
	EXPECT_FALSE(parameterDerivative2.isValid());

	const double one = 1.0;
	FieldConstant fieldConstant = zinc.fm.createFieldConstant(1, &one);
	EXPECT_TRUE(fieldConstant.isValid());
	noFieldparameters = fieldConstant.getFieldparameters();
	EXPECT_FALSE(noFieldparameters.isValid());
}
