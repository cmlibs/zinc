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
#include <zinc/field.h>
#include <zinc/fieldconstant.h>
#include <zinc/optimisation.h>

#include "zinctestsetupcpp.hpp"
#include <zinc/field.hpp>
#include <zinc/fieldconstant.hpp>
#include <zinc/optimisation.hpp>

TEST(cmzn_optimisation, valid_args)
{
	ZincTestSetup zinc;
	int result;

	cmzn_optimisation_id optimisation = cmzn_fieldmodule_create_optimisation(zinc.fm);
	EXPECT_NE(static_cast<cmzn_optimisation_id>(0), optimisation);

	EXPECT_EQ(CMZN_OPTIMISATION_METHOD_QUASI_NEWTON, cmzn_optimisation_get_method(optimisation));
	EXPECT_EQ(OK, result = cmzn_optimisation_set_method(optimisation, CMZN_OPTIMISATION_METHOD_LEAST_SQUARES_QUASI_NEWTON));
	EXPECT_EQ(CMZN_OPTIMISATION_METHOD_LEAST_SQUARES_QUASI_NEWTON, cmzn_optimisation_get_method(optimisation));

	cmzn_optimisation_destroy(&optimisation);
}

TEST(zincOptimisation, valid_args)
{
	ZincTestSetupCpp zinc;
	int result;

	Optimisation optimisation = zinc.fm.createOptimisation();
	EXPECT_TRUE(optimisation.isValid());

	EXPECT_EQ(Optimisation::METHOD_QUASI_NEWTON, optimisation.getMethod());
	EXPECT_EQ(OK, result = optimisation.setMethod(Optimisation::METHOD_LEAST_SQUARES_QUASI_NEWTON));
	EXPECT_EQ(Optimisation::METHOD_LEAST_SQUARES_QUASI_NEWTON, optimisation.getMethod());
}
