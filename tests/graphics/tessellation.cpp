/*
 * Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <cmlibs/zinc/status.h>
#include <cmlibs/zinc/core.h>
#include <cmlibs/zinc/tessellation.h>

#include "cmlibs/zinc/tessellation.hpp"

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

#include "test_resources.h"

TEST(cmzn_tessellationmodule_api, valid_args)
{
	ZincTestSetup zinc;

	cmzn_tessellationmodule_id tm = cmzn_context_get_tessellationmodule(zinc.context);
	EXPECT_NE(static_cast<cmzn_tessellationmodule *>(0), tm);

	int result = cmzn_tessellationmodule_begin_change(tm);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_tessellation_id tessellation = cmzn_tessellationmodule_get_default_tessellation(tm);
	EXPECT_NE(static_cast<cmzn_tessellation *>(0), tessellation);
	int value;
	result = cmzn_tessellation_get_minimum_divisions(tessellation, 1, &value);
	EXPECT_EQ(1, result);
	EXPECT_EQ(1, value);
	result = cmzn_tessellation_get_refinement_factors(tessellation, 1, &value);
	EXPECT_EQ(1, result);
	EXPECT_EQ(6, value);
	result = cmzn_tessellation_get_circle_divisions(tessellation);
	EXPECT_EQ(12, result);
	cmzn_tessellation_destroy(&tessellation);

	// following should destroy default tessellation as not managed and not used
	// otherwise it isn't possible to create a tessellation named "default" below
	result = cmzn_tessellationmodule_set_default_tessellation(tm, 0);
	EXPECT_EQ(CMZN_OK, result);

	tessellation = cmzn_tessellationmodule_get_default_points_tessellation(tm);
	EXPECT_NE(static_cast<cmzn_tessellation *>(0), tessellation);
	result = cmzn_tessellation_get_minimum_divisions(tessellation, 1, &value);
	EXPECT_EQ(1, result);
	EXPECT_EQ(1, value);
	result = cmzn_tessellation_get_refinement_factors(tessellation, 1, &value);
	EXPECT_EQ(1, result);
	EXPECT_EQ(1, value);
	result = cmzn_tessellation_get_circle_divisions(tessellation);
	EXPECT_EQ(12, result);
	cmzn_tessellation_destroy(&tessellation);

	tessellation = cmzn_tessellationmodule_create_tessellation(tm);
	EXPECT_NE(static_cast<cmzn_tessellation *>(0), tessellation);

	result = cmzn_tessellation_set_name(tessellation, "new_default");
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_tessellationmodule_end_change(tm);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_tessellationmodule_set_default_tessellation(tm, tessellation);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_tessellation_set_managed(tessellation, 1);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_tessellation_id temp_tessellation = cmzn_tessellationmodule_get_default_tessellation(tm);
	EXPECT_EQ(tessellation, temp_tessellation);
	cmzn_tessellation_destroy(&temp_tessellation);

	temp_tessellation = cmzn_tessellationmodule_find_tessellation_by_name(tm, "new_default");
	EXPECT_EQ(tessellation, temp_tessellation);
	cmzn_tessellation_destroy(&temp_tessellation);

	cmzn_tessellation_destroy(&tessellation);

	tessellation = cmzn_tessellationmodule_get_default_tessellation(tm);
	EXPECT_NE(static_cast<cmzn_tessellation *>(0), tessellation);
	cmzn_tessellation_destroy(&tessellation);

	cmzn_tessellationmodule_destroy(&tm);
}

TEST(cmzn_tessellationmodule_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	Tessellationmodule tm = zinc.context.getTessellationmodule();
	EXPECT_TRUE(tm.isValid());

	int result = tm.beginChange();
	EXPECT_EQ(CMZN_OK, result);

	Tessellation tessellation = tm.getDefaultTessellation();
	EXPECT_TRUE(tessellation.isValid());
	int value;
	result = tessellation.getMinimumDivisions(1, &value);
	EXPECT_EQ(1, result);
	EXPECT_EQ(1, value);
	result = tessellation.getRefinementFactors(1, &value);
	EXPECT_EQ(1, result);
	EXPECT_EQ(6, value);
	result = tessellation.getCircleDivisions();
	EXPECT_EQ(12, result);

	// following should destroy default tessellation as not managed and not used
	// otherwise it isn't possible to create a tessellation named "default" below
	tessellation = Tessellation();
	result = tm.setDefaultTessellation(tessellation);
	EXPECT_EQ(CMZN_OK, result);

	tessellation = tm.getDefaultPointsTessellation();
	EXPECT_TRUE(tessellation.isValid());
	result = tessellation.getMinimumDivisions(1, &value);
	EXPECT_EQ(1, result);
	EXPECT_EQ(1, value);
	result = tessellation.getRefinementFactors(1, &value);
	EXPECT_EQ(1, result);
	EXPECT_EQ(1, value);
	result = tessellation.getCircleDivisions();
	EXPECT_EQ(12, result);

	tessellation = tm.createTessellation();
	EXPECT_TRUE(tessellation.isValid());

	result = tessellation.setName("new_default");
	EXPECT_EQ(CMZN_OK, result);

	result = tm.endChange();
	EXPECT_EQ(CMZN_OK, result);

	result = tm.setDefaultTessellation( tessellation);
	EXPECT_EQ(CMZN_OK, result);

	result = tessellation.setManaged(1);
	EXPECT_EQ(CMZN_OK, result);

	Tessellation tempTessellation = tm.getDefaultTessellation();
	EXPECT_EQ(tessellation.getId(), tempTessellation.getId());

	tempTessellation = tm.findTessellationByName("new_default");
	EXPECT_EQ(tessellation.getId(), tempTessellation.getId());

	tessellation = tm.getDefaultTessellation();
	EXPECT_TRUE(tessellation.isValid());
}

TEST(cmzn_tessellation_api, valid_args)
{
	ZincTestSetup zinc;

	cmzn_tessellationmodule_id tm = cmzn_context_get_tessellationmodule(zinc.context);
	EXPECT_NE(static_cast<cmzn_tessellationmodule *>(0), tm);

	int result = cmzn_tessellationmodule_begin_change(tm);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_tessellation_id tessellation = cmzn_tessellationmodule_create_tessellation(tm);
	EXPECT_NE(static_cast<cmzn_tessellation *>(0), tessellation);

	result = cmzn_tessellation_set_name(tessellation, "new_default");
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_tessellationmodule_end_change(tm);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_tessellation_set_circle_divisions(tessellation, 10);
	EXPECT_EQ(CMZN_OK, result);
	result = cmzn_tessellation_get_circle_divisions(tessellation);
	EXPECT_EQ(10, result);

	result = cmzn_tessellation_set_managed(tessellation, 1);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_tessellation_is_managed(tessellation);
	EXPECT_EQ(1, result);

	const int inValues[3] = { 2, 3, 4 };
	int outValues[3];

	result = cmzn_tessellation_set_minimum_divisions(tessellation, 3, &inValues[0]);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_tessellation_get_minimum_divisions(tessellation, 3, &outValues[0]);
	EXPECT_EQ(3, result);
	for (int i = 0; i < 3; ++i)
		EXPECT_EQ(inValues[i], outValues[i]);

	result = cmzn_tessellation_set_refinement_factors(tessellation, 3, &inValues[0]);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_tessellation_get_refinement_factors(tessellation, 3, &outValues[0]);
	EXPECT_EQ(3, result);
	for (int i = 0; i < 3; ++i)
		EXPECT_EQ(inValues[i], outValues[i]);

	cmzn_tessellation_destroy(&tessellation);

	cmzn_tessellationmodule_destroy(&tm);
}

TEST(cmzn_tessellation_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	Tessellationmodule tm = zinc.context.getTessellationmodule();
	EXPECT_TRUE(tm.isValid());

	int result = tm.beginChange();
	EXPECT_EQ(CMZN_OK, result);

	Tessellation tessellation = tm.createTessellation();
	EXPECT_TRUE(tessellation.isValid());

	result = tessellation.setName("new_default");
	EXPECT_EQ(CMZN_OK, result);

	result = tm.endChange();
	EXPECT_EQ(CMZN_OK, result);

	result = tessellation.setCircleDivisions(10);
	EXPECT_EQ(CMZN_OK, result);
	result = tessellation.getCircleDivisions();
	EXPECT_EQ(10, result);

	result = tessellation.setManaged(true);
	EXPECT_EQ(CMZN_OK, result);

	EXPECT_TRUE(tessellation.isManaged());

	const int inValues[3] = { 2, 3, 4 };
	int outValues[3];

	result = tessellation.setMinimumDivisions(3, &inValues[0]);
	EXPECT_EQ(CMZN_OK, result);

	result = tessellation.getMinimumDivisions(3, &outValues[0]);
	EXPECT_EQ(3, result);
	for (int i = 0; i < 3; ++i)
		EXPECT_EQ(inValues[i], outValues[i]);

	result = tessellation.setRefinementFactors(3, &inValues[0]);
	EXPECT_EQ(CMZN_OK, result);

	result = tessellation.getRefinementFactors(3, &outValues[0]);
	EXPECT_EQ(3, result);
	for (int i = 0; i < 3; ++i)
		EXPECT_EQ(inValues[i], outValues[i]);

}


TEST(cmzn_tessellation_api, description_io_cpp)
{
	ZincTestSetupCpp zinc;

	Tessellationmodule tm = zinc.context.getTessellationmodule();
	EXPECT_TRUE(tm.isValid());

    std::string stringBuffer = fileContents("graphics/tessellation_description.json");
    EXPECT_FALSE(stringBuffer.empty());

    EXPECT_EQ(CMZN_OK, tm.readDescription(stringBuffer.c_str()));

	Tessellation tessellation = tm.findTessellationByName("default");
	EXPECT_TRUE(tessellation.isValid());
	EXPECT_TRUE(tessellation.isManaged());

	tessellation = tm.findTessellationByName("default_points");
	EXPECT_TRUE(tessellation.isValid());
	EXPECT_TRUE(tessellation.isManaged());

	tessellation = tm.findTessellationByName("new_default");
	EXPECT_TRUE(tessellation.isValid());
	EXPECT_TRUE(tessellation.isManaged());

	char *name = tm.getDefaultTessellation().getName();
	EXPECT_EQ(0, strcmp("new_default", name));
	cmzn_deallocate(name);

	name = tm.getDefaultPointsTessellation().getName();
	EXPECT_EQ(0, strcmp("default", name));
	cmzn_deallocate(name);

	EXPECT_EQ(10, tessellation.getCircleDivisions());

	int intValues[3];
	int returnedValue = 0;

	returnedValue = tessellation.getMinimumDivisions(2, &intValues[0]);
	EXPECT_EQ(2, returnedValue);
	EXPECT_EQ(2, intValues[0]);
	EXPECT_EQ(6, intValues[1]);

	returnedValue = tessellation.getRefinementFactors(3, &intValues[0]);
	EXPECT_EQ(3, returnedValue);
	EXPECT_EQ(3, intValues[0]);
	EXPECT_EQ(5, intValues[1]);
	EXPECT_EQ(7, intValues[2]);

	char *return_string = tm.writeDescription();
	EXPECT_TRUE(return_string != 0);
	cmzn_deallocate(return_string);
}


TEST(ZincTessellationiterator, iteration)
{
	ZincTestSetupCpp zinc;

	Tessellationmodule tm = zinc.context.getTessellationmodule();
	EXPECT_TRUE(tm.isValid());
	Tessellation defaultTessellation = tm.getDefaultTessellation();
	EXPECT_TRUE(defaultTessellation.isValid());
	Tessellation defaultPointsTessellation = tm.getDefaultPointsTessellation();
	EXPECT_TRUE(defaultPointsTessellation.isValid());

	Tessellation zzz = tm.createTessellation();
	EXPECT_TRUE(zzz.isValid());
	EXPECT_EQ(OK, zzz.setName("zzz"));
	char *name = zzz.getName();
	EXPECT_STREQ("zzz", name);
	cmzn_deallocate(name);
	name = 0;

	Tessellation aaa = tm.createTessellation();
	EXPECT_TRUE(aaa.isValid());
	EXPECT_EQ(OK, aaa.setName("aaa"));

	Tessellation aab = tm.createTessellation();
	EXPECT_TRUE(aab.isValid());
	EXPECT_EQ(OK, aab.setName("aab"));

	Tessellationiterator iter = tm.createTessellationiterator();
	EXPECT_TRUE(iter.isValid());
	Tessellation g;
	EXPECT_EQ(aaa, g = iter.next());
	EXPECT_EQ(aab, g = iter.next());
	EXPECT_EQ(defaultTessellation, g = iter.next());
	EXPECT_EQ(defaultPointsTessellation, g = iter.next());
	EXPECT_EQ(zzz, g = iter.next());
	g = iter.next();
	EXPECT_FALSE(g.isValid());
}

class TessellationcallbackRecordChange : public Tessellationmodulecallback
{
	int changeFlags;

	virtual void operator()(const Tessellationmoduleevent &tessellationmoduleevent)
	{
		this->changeFlags = tessellationmoduleevent.getSummaryTessellationChangeFlags();
	}

public:
	TessellationcallbackRecordChange() :
		Tessellationmodulecallback(),
			changeFlags(Tessellation::CHANGE_FLAG_NONE)
	{ }

	void clear()
	{
		changeFlags = Tessellation::CHANGE_FLAG_NONE;
	}

	int getChangeSummary() const
	{
		return this->changeFlags;
	}

};

TEST(ZincTessellationmodulenotifier, changeCallback)
{
	ZincTestSetupCpp zinc;
	int result;

	Tessellationmodule tm = zinc.context.getTessellationmodule();
	EXPECT_TRUE(tm.isValid());

	Tessellationmodulenotifier tessellationmodulenotifier = tm.createTessellationmodulenotifier();
	EXPECT_TRUE(tessellationmodulenotifier.isValid());

	TessellationcallbackRecordChange callback;
	EXPECT_EQ(CMZN_OK, result = tessellationmodulenotifier.setCallback(callback));

	Tessellation tessellation = tm.getDefaultTessellation();
	EXPECT_TRUE(tessellation.isValid());

	Tessellation new_tessellation = tm.createTessellation();
	EXPECT_TRUE(new_tessellation.isValid());
	EXPECT_EQ(Tessellation::CHANGE_FLAG_ADD, callback.getChangeSummary());

	new_tessellation = Tessellation();
	EXPECT_FALSE(new_tessellation.isValid());
	EXPECT_EQ(Tessellation::CHANGE_FLAG_REMOVE, callback.getChangeSummary());

	EXPECT_EQ(CMZN_OK, tessellation.setCircleDivisions(6));
	EXPECT_EQ(Tessellation::CHANGE_FLAG_FULL_RESULT |Tessellation::CHANGE_FLAG_DEFINITION,
		callback.getChangeSummary());
}
