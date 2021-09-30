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
#include "opencmiss/zinc/fieldconstant.hpp"

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

	EXPECT_STREQ("", zinc.root_region.getPath());
	Region bob = zinc.root_region.createChild("bob");
	EXPECT_TRUE(bob.isValid());
	EXPECT_STREQ("bob", bob.getPath());
	EXPECT_STREQ("..", zinc.root_region.getRelativePath(bob));

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
	EXPECT_EQ(bob, zinc.root_region.findSubregionAtPath("bob"));

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

	// test can create and find subregions from relative paths
	Region tom = harry.createSubregion("../../tom");
	EXPECT_TRUE(tom.isValid());
	EXPECT_EQ(tom, harry.findSubregionAtPath("../../tom"));
	EXPECT_EQ(tom, zinc.root_region.findSubregionAtPath("bob/tom"));
	EXPECT_EQ(tom, joe.findSubregionAtPath("../tom"));
	EXPECT_EQ(tom, joe.findSubregionAtPath("/../tom"));
	EXPECT_EQ(tom, joe.findSubregionAtPath("../tom/"));
	EXPECT_EQ(zinc.root_region, bob.findSubregionAtPath(".."));
	// test finding at invalid relative paths
	EXPECT_FALSE(bob.findSubregionAtPath(".").isValid());
	EXPECT_FALSE(bob.findSubregionAtPath("...").isValid());
	EXPECT_FALSE(bob.findSubregionAtPath("../..").isValid());
	EXPECT_FALSE(harry.findSubregionAtPath("../harry/none").isValid());
	EXPECT_FALSE(harry.createSubregion("../../tom").isValid());  // already exists so should not succeed

	// test root
	EXPECT_EQ(zinc.root_region, harry.getRoot());
	Region independent = zinc.root_region.createRegion();
	EXPECT_TRUE(independent.isValid());
	EXPECT_EQ(independent, independent.getRoot());

	// test paths
	EXPECT_STREQ("bob/wills/harry", harry.getPath());
	EXPECT_STREQ("wills/harry", harry.getRelativePath(bob));
	EXPECT_STREQ("harry", harry.getRelativePath(bob.findChildByName("wills")));
	EXPECT_STREQ("", harry.getRelativePath(harry));
	EXPECT_STREQ("../wills/harry", harry.getRelativePath(fred));
	EXPECT_STREQ("../../fred", fred.getRelativePath(harry));
	EXPECT_STREQ("", joe.getRelativePath(joe));
	EXPECT_STREQ("../..", bob.getRelativePath(harry));
	EXPECT_STREQ("../../..", zinc.root_region.getRelativePath(harry));

	// test invalid paths
	EXPECT_EQ(nullptr, independent.getRelativePath(zinc.root_region));
	EXPECT_EQ(nullptr, zinc.root_region.getRelativePath(independent));
	EXPECT_EQ(nullptr, zinc.root_region.getRelativePath(Region()));

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

TEST(ZincRegion, getContext)
{
	ZincTestSetupCpp zinc;

	Context context = zinc.root_region.getContext();
	EXPECT_EQ(zinc.context, context);
}

TEST(ZincRegion, append_insert_region)
{
	ZincTestSetupCpp zinc;

	Region r1 = zinc.root_region.createRegion();
	EXPECT_TRUE(r1.isValid());
	Region r2 = zinc.root_region.createRegion();
	EXPECT_TRUE(r2.isValid());

	EXPECT_EQ(ERROR_ARGUMENT, zinc.root_region.appendChild(r1));
	EXPECT_EQ(OK, r1.setName("r1"));
	EXPECT_EQ(OK, zinc.root_region.appendChild(r1));
	EXPECT_EQ(OK, r2.setName("r1"));
	EXPECT_EQ(ERROR_ARGUMENT, zinc.root_region.appendChild(r2));
	EXPECT_EQ(ERROR_ARGUMENT, zinc.root_region.insertChildBefore(r2, r1));
	EXPECT_EQ(OK, r2.setName("r2"));
	EXPECT_EQ(OK, zinc.root_region.appendChild(r2));
	EXPECT_EQ(r1, zinc.root_region.getFirstChild());
	EXPECT_EQ(OK, zinc.root_region.insertChildBefore(r2, r1));
	EXPECT_EQ(r2, zinc.root_region.getFirstChild());

	Context otherContext = Context("other");
	Region or1 = otherContext.createRegion();
	EXPECT_TRUE(or1.isValid());
	EXPECT_EQ(OK, or1.setName("or1"));

	EXPECT_EQ(ERROR_ARGUMENT_CONTEXT, zinc.root_region.appendChild(or1));
	EXPECT_EQ(ERROR_ARGUMENT_CONTEXT, zinc.root_region.insertChildBefore(or1, r2));
}

class FieldmodulecallbackRecordChange : public Fieldmodulecallback
{
public:
	Fieldmoduleevent lastEvent;

	FieldmodulecallbackRecordChange()
	{ }

	virtual void operator()(const Fieldmoduleevent& event)
	{
		this->lastEvent = event;
	}
};

// Note Zinc currently doesn't notify of region tree changes
// For now just test that fieldmodule change is active
TEST(ZincRegion, ChangeManager)
{
	ZincTestSetupCpp zinc;
	int change;

	Region region = zinc.fm.getRegion();
	Fieldmodulenotifier notifier = zinc.fm.createFieldmodulenotifier();
	EXPECT_TRUE(notifier.isValid());
	FieldmodulecallbackRecordChange recordChange;
	EXPECT_EQ(RESULT_OK, notifier.setCallback(recordChange));
	{
		ChangeManager<Region> changeRegion(region);
		const double valueOne = 1.0;
		FieldConstant one = zinc.fm.createFieldConstant(1, &valueOne);
		EXPECT_EQ(Field::CHANGE_FLAG_NONE, change = recordChange.lastEvent.getSummaryFieldChangeFlags());
		EXPECT_EQ(RESULT_OK, one.setManaged(true));
		EXPECT_EQ(Field::CHANGE_FLAG_NONE, change = recordChange.lastEvent.getSummaryFieldChangeFlags());
	}
	change = recordChange.lastEvent.getSummaryFieldChangeFlags();
	EXPECT_EQ(Field::CHANGE_FLAG_ADD, change = recordChange.lastEvent.getSummaryFieldChangeFlags());
}

// Note Zinc currently doesn't notify of region tree changes
TEST(ZincRegion, HierarchicalChangeManager)
{
	ZincTestSetupCpp zinc;
	int change;

	Region region = zinc.fm.getRegion();
	Region child = region.createChild("child");
	EXPECT_TRUE(child.isValid());
	Fieldmodule childFm = child.getFieldmodule();
	EXPECT_TRUE(childFm.isValid());
	Fieldmodulenotifier notifier = childFm.createFieldmodulenotifier();
	EXPECT_TRUE(notifier.isValid());
	FieldmodulecallbackRecordChange recordChange;
	EXPECT_EQ(RESULT_OK, notifier.setCallback(recordChange));
	{
		HierarchicalChangeManager<Region> changeRegion(region);
		const double valueOne = 1.0;
		FieldConstant one = childFm.createFieldConstant(1, &valueOne);
		EXPECT_EQ(Field::CHANGE_FLAG_NONE, change = recordChange.lastEvent.getSummaryFieldChangeFlags());
		EXPECT_EQ(RESULT_OK, one.setManaged(true));
		EXPECT_EQ(Field::CHANGE_FLAG_NONE, change = recordChange.lastEvent.getSummaryFieldChangeFlags());
	}
	EXPECT_EQ(Field::CHANGE_FLAG_ADD, change = recordChange.lastEvent.getSummaryFieldChangeFlags());
}
