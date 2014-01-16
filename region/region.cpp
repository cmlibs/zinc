/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

TEST(cmzn_region, build_tree)
{
	ZincTestSetup zinc;
	int result;
	cmzn_region_id tmp = 0;

	cmzn_region_id bob = cmzn_region_create_child(zinc.root_region, "bob");
	EXPECT_NE(static_cast<cmzn_region_id>(0), bob);

	cmzn_region_id alf = cmzn_region_create_child(bob, "alf");
	EXPECT_NE(static_cast<cmzn_region_id>(0), alf);
	EXPECT_EQ(bob, tmp = cmzn_region_get_parent(alf));
	cmzn_region_destroy(&tmp);

	cmzn_region_id fred = cmzn_region_create_region(zinc.root_region);
	EXPECT_NE(static_cast<cmzn_region_id>(0), bob);
	EXPECT_EQ(CMZN_OK, result = cmzn_region_set_name(fred, "fred"));
	EXPECT_EQ(CMZN_OK, result = cmzn_region_append_child(bob, fred));

	EXPECT_EQ(bob, tmp = cmzn_region_get_first_child(zinc.root_region));
	cmzn_region_destroy(&tmp);
	EXPECT_EQ(static_cast<cmzn_region_id>(0), cmzn_region_get_next_sibling(bob));
	EXPECT_EQ(bob, tmp = cmzn_region_find_child_by_name(zinc.root_region, "bob"));
	cmzn_region_destroy(&tmp);

	EXPECT_EQ(alf, tmp = cmzn_region_get_first_child(bob));
	cmzn_region_destroy(&tmp);
	EXPECT_EQ(fred, tmp = cmzn_region_get_next_sibling(alf));
	cmzn_region_destroy(&tmp);
	EXPECT_EQ(static_cast<cmzn_region_id>(0), cmzn_region_get_next_sibling(fred));
	EXPECT_EQ(alf, tmp = cmzn_region_get_previous_sibling(fred));
	cmzn_region_destroy(&tmp);
	EXPECT_EQ(fred, tmp = cmzn_region_find_subregion_at_path(zinc.root_region, "bob/fred"));

	EXPECT_EQ(CMZN_OK, result = cmzn_region_remove_child(bob, fred));
	EXPECT_EQ(static_cast<cmzn_region_id>(0), tmp = cmzn_region_find_subregion_at_path(zinc.root_region, "bob/fred"));
	EXPECT_EQ(CMZN_OK, result = cmzn_region_insert_child_before(bob, fred, /*before*/alf));
	EXPECT_EQ(fred, tmp = cmzn_region_get_first_child(bob));
	cmzn_region_destroy(&tmp);
	EXPECT_EQ(alf, tmp = cmzn_region_get_next_sibling(fred));
	cmzn_region_destroy(&tmp);

	cmzn_region_id joe = cmzn_region_create_subregion(zinc.root_region, "bob/joe");
	EXPECT_NE(static_cast<cmzn_region_id>(0), alf);
	EXPECT_EQ(joe, tmp = cmzn_region_get_next_sibling(alf));
	cmzn_region_destroy(&tmp);
	EXPECT_EQ(static_cast<cmzn_region_id>(0), cmzn_region_create_subregion(zinc.root_region, "bob/joe")); // exists already

	// test can create intermediate subregions
	cmzn_region_id harry = cmzn_region_create_subregion(bob, "wills/harry");
	EXPECT_EQ(harry, tmp = cmzn_region_find_subregion_at_path(zinc.root_region, "bob/wills/harry"));
	cmzn_region_destroy(&tmp);

	EXPECT_TRUE(cmzn_region_contains_subregion(zinc.root_region, joe));
	EXPECT_TRUE(cmzn_region_contains_subregion(bob, fred));
	EXPECT_FALSE(cmzn_region_contains_subregion(alf, bob));

	cmzn_region_destroy(&harry);
	cmzn_region_destroy(&joe);
	cmzn_region_destroy(&alf);
	cmzn_region_destroy(&fred);
	cmzn_region_destroy(&bob);
}

TEST(ZincRegion, build_tree)
{
	ZincTestSetupCpp zinc;
	int result;

	Region bob = zinc.root_region.createChild("bob");
	EXPECT_TRUE(bob.isValid());

	Region alf = bob.createChild("alf");
	EXPECT_TRUE(alf.isValid());
	EXPECT_EQ(bob, alf.getParent());

	Region fred = zinc.root_region.createRegion();
	EXPECT_TRUE(fred.isValid());
	EXPECT_EQ(CMZN_OK, result = fred.setName("fred"));
	EXPECT_EQ(CMZN_OK, result = bob.appendChild(fred));

	EXPECT_EQ(bob, zinc.root_region.getFirstChild());
	EXPECT_EQ(Region(), bob.getNextSibling());
	EXPECT_EQ(bob, zinc.root_region.findChildByName("bob"));
	
	EXPECT_EQ(alf, bob.getFirstChild());
	EXPECT_EQ(fred, alf.getNextSibling());
	EXPECT_EQ(Region(), fred.getNextSibling());
	EXPECT_EQ(alf, fred.getPreviousSibling());
	EXPECT_EQ(fred, zinc.root_region.findSubregionAtPath("bob/fred"));

	EXPECT_EQ(CMZN_OK, result = bob.removeChild(fred));
	EXPECT_EQ(Region(), zinc.root_region.findSubregionAtPath("bob/fred"));
	EXPECT_EQ(CMZN_OK, result = bob.insertChildBefore(fred, /*before*/alf));
	EXPECT_EQ(fred, bob.getFirstChild());
	EXPECT_EQ(alf, fred.getNextSibling());

	Region joe = zinc.root_region.createSubregion("bob/joe");
	EXPECT_TRUE(joe.isValid());
	EXPECT_EQ(joe, alf.getNextSibling());
	EXPECT_FALSE(zinc.root_region.createSubregion("bob/joe").isValid()); // exists already

	// test can create intermediate subregions
	Region harry = bob.createSubregion("wills/harry");
	EXPECT_EQ(harry, zinc.root_region.findSubregionAtPath("bob/wills/harry"));

	EXPECT_TRUE(zinc.root_region.containsSubregion(joe));
	EXPECT_TRUE(bob.containsSubregion(fred));
	EXPECT_FALSE(alf.containsSubregion(bob));
}

TEST(cmzn_region, fieldmodule_get_region)
{
	ZincTestSetup zinc;

	cmzn_region_id region = cmzn_fieldmodule_get_region(zinc.fm);
	EXPECT_EQ(zinc.root_region, region);
	cmzn_region_destroy(&region);
}

TEST(ZincRegion, Fieldmodule_getRegion)
{
	ZincTestSetupCpp zinc;

	Region region = zinc.fm.getRegion();
	EXPECT_EQ(zinc.root_region, region);
	Fieldmodule fm = region.getFieldmodule();
	EXPECT_EQ(zinc.fm, fm);
}
