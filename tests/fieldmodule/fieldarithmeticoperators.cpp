/*
 * Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <opencmiss/zinc/field.hpp>
#include <opencmiss/zinc/fieldarithmeticoperators.hpp>
#include <opencmiss/zinc/fieldcache.hpp>
#include <opencmiss/zinc/fieldconstant.hpp>

#include "zinctestsetupcpp.hpp"

TEST(ZincFieldAdd, scalar_broadcast)
{
	ZincTestSetupCpp zinc;

	const double values_in[3] = { 1.0, 2.5, -3.0 };
	FieldConstant fieldConstant = zinc.fm.createFieldConstant(3, values_in);
	EXPECT_TRUE(fieldConstant.isValid());
	const double scalar_in = 1.0;
	FieldConstant fieldScalar = zinc.fm.createFieldConstant(1, &scalar_in);
	EXPECT_TRUE(fieldScalar.isValid());
	FieldAdd fieldAdd1 = fieldConstant + fieldScalar;
	EXPECT_TRUE(fieldAdd1.isValid());
	FieldAdd fieldAdd2 = fieldScalar + fieldConstant;
	EXPECT_TRUE(fieldAdd2.isValid());

	Fieldcache fieldcache = zinc.fm.createFieldcache();
	double values_out[3];
	EXPECT_EQ(RESULT_OK, fieldAdd1.evaluateReal(fieldcache,3, values_out));
	for (int c = 0; c < 3; ++c)
		EXPECT_DOUBLE_EQ(values_in[c] + scalar_in, values_out[c]);
	EXPECT_EQ(RESULT_OK, fieldAdd2.evaluateReal(fieldcache, 3, values_out));
	for (int c = 0; c < 3; ++c)
		EXPECT_DOUBLE_EQ(values_in[c] + scalar_in, values_out[c]);
}
