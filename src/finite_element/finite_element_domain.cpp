/**
 * FILE : finite_element_domain.cpp
 *
 * Abstract base class for finite element domains built on a set of labels.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "finite_element/finite_element_domain.hpp"
#include "general/debug.h"
#include "general/message.h"


FE_domain::FE_domain(FE_region *fe_region, int dimensionIn) :
	RefCounted(),
	fe_region(fe_region),
	dimension(dimensionIn),
	changeLog(nullptr),
	destroyedLabelsGroup(nullptr)
{
	// concrete derived classes must call this virtual function in their constructors:
	//this->createChangeLog();
}

FE_domain::~FE_domain()
{
	// concrete derived classes must call this in their destructors
	// to avoid messages during their cleanup:
	//cmzn::Deaccess(this->changeLog);
}

/** Private: assumes current change log pointer is valid or nullptr */
void FE_domain::createChangeLog()
{
	cmzn::Deaccess(this->changeLog);
	this->changeLog = DsLabelsChangeLog::create(&this->labels);
	if (!this->changeLog)
		display_message(ERROR_MESSAGE, "FE_domain::createChangeLog.  Failed to create change log");
}

/** Assumes called by FE_region destructor, and change notification is disabled. */
void FE_domain::detach_from_FE_region()
{
	this->fe_region = 0;
}

DsLabelsGroup *FE_domain::createLabelsGroup()
{
	return DsLabelsGroup::create(&this->labels); // GRC dodgy taking address here
}

// Only to be called by FE_region_clear, or when all domains already removed
// to reclaim memory in labels and mapped arrays
void FE_domain::clear()
{
	this->labels.clear();
}

DsLabelsChangeLog *FE_domain::extractChangeLog()
{
	// take access count of changelog when extracting
	DsLabelsChangeLog *returnChangeLog = this->changeLog;
	this->changeLog = nullptr;
	this->createChangeLog();
	return returnChangeLog;
}

void FE_domain::list_btree_statistics()
{
	this->labels.list_storage_details();
}
