/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __ZINCTESTSETUP_HPP__
#define __ZINCTESTSETUP_HPP__

#include <gtest/gtest.h>

#include <zinc/status.h>
#include <zinc/context.h>
#include <zinc/region.h>
#include <zinc/fieldmodule.h>

class ZincTestSetup
{
public:
	cmzn_context_id context;
	cmzn_region_id root_region;
	cmzn_fieldmodule_id fm;

	ZincTestSetup() :
		context(cmzn_context_create("test")),
		root_region(cmzn_context_get_default_region(context)),
		fm(cmzn_region_get_fieldmodule(root_region))
	{
		EXPECT_NE(static_cast<cmzn_fieldmodule *>(0), fm);
	}

	~ZincTestSetup()
	{
		cmzn_fieldmodule_destroy(&fm);
		cmzn_region_destroy(&root_region);
		cmzn_context_destroy(&context);
	}
};

#endif // __ZINCTESTSETUP_HPP__
