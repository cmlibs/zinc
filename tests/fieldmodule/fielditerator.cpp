/*
 * Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <cmlibs/zinc/field.h>
#include <cmlibs/zinc/fieldarithmeticoperators.h>
#include <cmlibs/zinc/fieldcache.h>
#include <cmlibs/zinc/fieldconstant.h>
#include <cmlibs/zinc/status.h>

#include <cmlibs/zinc/field.hpp>
#include <cmlibs/zinc/fieldarithmeticoperators.hpp>
#include <cmlibs/zinc/fieldcache.hpp>
#include <cmlibs/zinc/fieldconstant.hpp>
#include <cmlibs/zinc/status.hpp>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

TEST(cmzn_fielditerator, iteration)
{
	ZincTestSetup zinc;

	const double value1 = 2.0;
	cmzn_field_id joe = cmzn_fieldmodule_create_field_constant(zinc.fm, 1, &value1);
	EXPECT_NE((cmzn_field_id)0, joe);
	EXPECT_EQ(CMZN_OK, cmzn_field_set_name(joe, "joe"));

	const double value2 = 1.0;
	cmzn_field_id bob = cmzn_fieldmodule_create_field_constant(zinc.fm, 1, &value2);
	EXPECT_NE((cmzn_field_id)0, bob);
	EXPECT_EQ(CMZN_OK, cmzn_field_set_name(bob, "bob"));

	cmzn_field_id fred = cmzn_fieldmodule_create_field_add(zinc.fm, joe, bob);
	EXPECT_NE((cmzn_field_id)0, fred);
	EXPECT_EQ(CMZN_OK, cmzn_field_set_name(fred, "fred"));

	cmzn_fielditerator_id iter = cmzn_fieldmodule_create_fielditerator(zinc.fm);
	EXPECT_NE((cmzn_fielditerator_id)0, iter);
	cmzn_field_id f;
	f = cmzn_fielditerator_next(iter);
	EXPECT_EQ(bob, f);
	cmzn_field_destroy(&f);
	f = cmzn_fielditerator_next(iter);
	EXPECT_EQ(fred, f);
	cmzn_field_destroy(&f);
	f = cmzn_fielditerator_next(iter);
	EXPECT_EQ(joe, f);
	cmzn_field_destroy(&f);

	cmzn_fielditerator_destroy(&iter);

	cmzn_field_destroy(&joe);
	cmzn_field_destroy(&bob);
	cmzn_field_destroy(&fred);
}

TEST(ZincFielditerator, iteration)
{
	ZincTestSetupCpp zinc;

	const double value1 = 2.0;
	Field joe = zinc.fm.createFieldConstant(1, &value1);
	EXPECT_TRUE(joe.isValid());
	EXPECT_EQ(OK, joe.setName("joe"));

	const double value2 = 1.0;
	Field bob = zinc.fm.createFieldConstant(1, &value2);
	EXPECT_TRUE(bob.isValid());
	EXPECT_EQ(OK, bob.setName("bob"));

	Field fred = zinc.fm.createFieldAdd(joe, bob);
	EXPECT_TRUE(fred.isValid());
	EXPECT_EQ(OK, fred.setName("fred"));

	Fielditerator iter = zinc.fm.createFielditerator();
	EXPECT_TRUE(iter.isValid());
	Field f;
	f = iter.next();
	EXPECT_EQ(bob, f);
	f = iter.next();
	EXPECT_EQ(fred, f);
	f = iter.next();
	EXPECT_EQ(joe, f);
}
