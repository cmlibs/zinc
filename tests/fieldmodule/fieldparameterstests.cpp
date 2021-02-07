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
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	Element element = mesh3d.findElementByIdentifier(1);
	EXPECT_TRUE(element.isValid());

	Fieldparameters fieldparameters = coordinates.getFieldparameters();
	EXPECT_TRUE(fieldparameters.isValid());
	EXPECT_EQ(24, fieldparameters.getNumberOfElementParameters(element));
	EXPECT_EQ(24, fieldparameters.getNumberOfParameters());

	Field fieldOut = fieldparameters.getField();
	EXPECT_TRUE(fieldOut.isValid());
	EXPECT_EQ(coordinates, fieldOut);
}

TEST(ZincFieldparameters, invalidAPI)
{
	ZincTestSetupCpp zinc;

	FieldFiniteElement fieldFiniteElement = zinc.fm.createFieldFiniteElement(1);
	EXPECT_TRUE(fieldFiniteElement.isValid());
	Fieldparameters fieldparameters = fieldFiniteElement.getFieldparameters();
	EXPECT_TRUE(fieldparameters.isValid());
	Field noField;
	EXPECT_FALSE(noField.isValid());
	Fieldparameters noFieldparameters = noField.getFieldparameters();
	EXPECT_FALSE(noFieldparameters.isValid());
	Field fieldOut = noFieldparameters.getField();
	EXPECT_FALSE(fieldOut.isValid());
	Element noElement;
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	Elementtemplate elementtemplate = mesh3d.createElementtemplate();
	EXPECT_EQ(RESULT_OK, elementtemplate.setElementShapeType(Element::SHAPE_TYPE_CUBE));
	Element element = mesh3d.createElement(1, elementtemplate);
	EXPECT_TRUE(element.isValid());
	EXPECT_EQ(-1, fieldparameters.getNumberOfElementParameters(noElement));
	EXPECT_EQ(-1, noFieldparameters.getNumberOfElementParameters(element));
	EXPECT_EQ(-1, noFieldparameters.getNumberOfElementParameters(noElement));
	EXPECT_EQ(-1, noFieldparameters.getNumberOfParameters());

	const double one = 1.0;
	FieldConstant fieldConstant = zinc.fm.createFieldConstant(1, &one);
	EXPECT_TRUE(fieldConstant.isValid());
	noFieldparameters = fieldConstant.getFieldparameters();
	EXPECT_FALSE(noFieldparameters.isValid());
}
