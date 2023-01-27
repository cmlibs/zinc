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

#include <opencmiss/zinc/result.h>
#include <opencmiss/zinc/context.h>

class ZincTestSetup
{
public:
	cmzn_context_id context;

	ZincTestSetup() :
        context(cmzn_context_create("test"))
	{
	}

	~ZincTestSetup()
	{
		cmzn_context_destroy(&context);
	}
};

#endif // __ZINCTESTSETUP_HPP__
