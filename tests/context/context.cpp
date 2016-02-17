/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include "opencmiss/zinc/core.h"
#include <zinc/region.hpp>
#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

TEST(cmzn_context, getVersion)
{
	ZincTestSetup zinc;

	int version[3];
	int result = CMZN_ERROR_ARGUMENT;
	const char *revision;

	EXPECT_EQ(CMZN_OK, result = cmzn_context_get_version(zinc.context, &version[0]));
	EXPECT_GE(version[0], 2);

	EXPECT_EQ(cmzn_context_get_revision(0), (const char *)0);
	EXPECT_STRNE(revision = cmzn_context_get_revision(zinc.context), "");
	char *versionString = cmzn_context_get_version_string(zinc.context);
	EXPECT_NE(versionString, (char *)0);
	cmzn_deallocate(versionString);
}

TEST(ZincContext, getVersion)
{
	ZincTestSetupCpp zinc;
	int version[3];

	int result = CMZN_ERROR_ARGUMENT;
	EXPECT_EQ(CMZN_OK, result = zinc.context.getVersion(&version[0]));
	EXPECT_GE(version[0], 2);

	EXPECT_STRNE(zinc.context.getRevision(), "");

	char *versionString = zinc.context.getVersionString();
	EXPECT_NE(versionString, (char *)0);
	cmzn_deallocate(versionString);
}

TEST(ZincContext, default_region)
{
	ZincTestSetupCpp zinc;

	Region r1 = zinc.context.getDefaultRegion();
	EXPECT_TRUE(r1.isValid());
	Region r2 = zinc.context.createRegion();
	EXPECT_TRUE(r2.isValid());
	EXPECT_EQ(OK, zinc.context.setDefaultRegion(r2));
	Region r3 = zinc.context.getDefaultRegion();
	EXPECT_EQ(r2, r3);

	Context nullContext;
	Region nullRegion;
	EXPECT_EQ(nullRegion, nullContext.getDefaultRegion());
	EXPECT_EQ(ERROR_ARGUMENT, nullContext.setDefaultRegion(r1));

	Context otherContext = Context("other");
	EXPECT_EQ(ERROR_ARGUMENT_CONTEXT, otherContext.setDefaultRegion(r1));
}
