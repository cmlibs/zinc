/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

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

	Fieldparameters fieldparameters = coordinates.getFieldparameters();
	EXPECT_TRUE(fieldparameters.isValid());

	EXPECT_EQ(-1, fieldparameters.getNumberOfParameters());  // not yet implemented

	Field fieldOut = fieldparameters.getField();
	EXPECT_TRUE(fieldOut.isValid());
	EXPECT_EQ(coordinates, fieldOut);
}

TEST(ZincFieldparameters, invalidAPI)
{
	ZincTestSetupCpp zinc;

	Field noField;
	EXPECT_FALSE(noField.isValid());
	Fieldparameters noFieldparameters = noField.getFieldparameters();
	EXPECT_FALSE(noFieldparameters.isValid());
	EXPECT_EQ(-1, noFieldparameters.getNumberOfParameters());
	Field fieldOut = noFieldparameters.getField();
	EXPECT_FALSE(fieldOut.isValid());

	const double one = 1.0;
	FieldConstant fieldConstant = zinc.fm.createFieldConstant(1, &one);
	EXPECT_TRUE(fieldConstant.isValid());
	noFieldparameters = fieldConstant.getFieldparameters();
	EXPECT_FALSE(noFieldparameters.isValid());
}
