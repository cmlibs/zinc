/*
 * Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <cmlibs/zinc/core.h>
#include <cmlibs/zinc/element.hpp>
#include <cmlibs/zinc/fieldmodule.hpp>
#include <cmlibs/zinc/region.hpp>
#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

TEST(cmzn_context, getVersion)
{
	ZincTestSetup zinc;

	int version[3];
	int result = CMZN_RESULT_ERROR_ARGUMENT;
	const char *revision;

	EXPECT_EQ(CMZN_RESULT_OK, result = cmzn_context_get_version(zinc.context, &version[0]));
	EXPECT_GE(version[0], 3);
	EXPECT_GE(version[1], 10);
	EXPECT_GE(version[2], 0);

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

	int result = RESULT_ERROR_ARGUMENT;
	EXPECT_EQ(RESULT_OK, result = zinc.context.getVersion(&version[0]));
	EXPECT_GE(version[0], 3);

	EXPECT_STRNE(zinc.context.getRevision(), "");

	char *versionString = zinc.context.getVersionString();
	EXPECT_NE(versionString, (char *)0);
	cmzn_deallocate(versionString);
}

TEST(ZincContext, getName)
{
	const char* nameIn = "test";
	Context context(nameIn);
	EXPECT_TRUE(context.isValid());
	char *nameOut = context.getName();
	EXPECT_STREQ(nameIn, nameOut);
	cmzn_deallocate(nameOut);
}

void testCreateFiniteElement(const Region &region)
{
	Fieldmodule fm = region.getFieldmodule();
	EXPECT_TRUE(fm.isValid());
	Mesh mesh = fm.findMeshByDimension(3);
	EXPECT_TRUE(mesh.isValid());
	Elementtemplate elementtemplate = mesh.createElementtemplate();
	EXPECT_TRUE(elementtemplate.isValid());
	EXPECT_EQ(RESULT_OK, elementtemplate.setElementShapeType(Element::SHAPE_TYPE_CUBE));
	EXPECT_EQ(RESULT_OK, mesh.defineElement(-1, elementtemplate));
}

TEST(ZincContext, default_region)
{
	ZincTestSetupCpp zinc;

	Region r1 = zinc.context.getDefaultRegion();
	EXPECT_TRUE(r1.isValid());
	testCreateFiniteElement(r1);
	Region r2 = zinc.context.createRegion();
	EXPECT_TRUE(r2.isValid());
	testCreateFiniteElement(r2);
	EXPECT_EQ(RESULT_OK, zinc.context.setDefaultRegion(r2));
	Region r3 = zinc.context.getDefaultRegion();
	EXPECT_EQ(r2, r3);

	// test fixed bug: creation of finite elements on region crashes after first region destroyed
	r1 = Region();
	testCreateFiniteElement(r2);

	Context nullContext;
	Region nullRegion;
	EXPECT_EQ(nullRegion, nullContext.getDefaultRegion());
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, nullContext.setDefaultRegion(r2));

    Context otherContext = Context("other");
    EXPECT_EQ(RESULT_ERROR_ARGUMENT_CONTEXT, otherContext.setDefaultRegion(r2));
}

TEST(cmzn_context, region_lifetime)
{
    cmzn_context_id context = cmzn_context_create("test");
    cmzn_region_id region = cmzn_context_create_region(context);
    cmzn_context_destroy(&context);
    cmzn_region_destroy(&region);
}
