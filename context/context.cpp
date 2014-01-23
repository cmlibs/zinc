/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include "zinc/core.h"
#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

TEST(cmzn_context, getVersion)
{
	ZincTestSetup zinc;

	int version[3];
	int result = CMZN_ERROR_ARGUMENT;

	EXPECT_EQ(CMZN_OK, result = cmzn_context_get_version(zinc.context, &version[0]));
	EXPECT_GE(version[0], 2);

	EXPECT_GE(result = cmzn_context_get_revision(zinc.context), 10000);
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

	EXPECT_GE(zinc.context.getRevision(), 10000);

	char *versionString = zinc.context.getVersionString();
	EXPECT_NE(versionString, (char *)0);
	cmzn_deallocate(versionString);
}
