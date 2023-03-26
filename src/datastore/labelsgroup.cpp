/**
 * FILE : datastore/labelsgroup.cpp
 * 
 * Implements a subset of a datastore labels set.
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "opencmiss/zinc/status.h"
#include "datastore/labelsgroup.hpp"

DsLabelsGroup::DsLabelsGroup(DsLabels *labelsIn) :
	cmzn::RefCounted(),
	labels(labelsIn),
	labelsCount(0),
	indexLimit(0)
{
};

DsLabelsGroup::~DsLabelsGroup()
{
	if (this->labels)
		this->labels->invalidateLabelIteratorsWithCondition(&(this->values));
}

DsLabelsGroup *DsLabelsGroup::create(DsLabels *labelsIn)
{
	return new DsLabelsGroup(labelsIn);
}

void DsLabelsGroup::swap(DsLabelsGroup& other)
{
	this->values.swap(other.values);
	int temp_labelsCount = this->labelsCount;
	int temp_indexLimit = this->indexLimit;
	this->labelsCount = other.labelsCount;
	this->indexLimit = other.indexLimit;
	other.labelsCount = temp_labelsCount;
	other.indexLimit = temp_indexLimit;
}

void DsLabelsGroup::clear()
{
	this->values.clear();
	this->labelsCount = 0;
	this->indexLimit = 0;
}

int DsLabelsGroup::removeGroup(DsLabelsGroup& otherGroup)
{
	// This could be made faster by working directly with data blocks
	int numberRemoved = 0;
	DsLabelIndex index = -1;  // So first increment gives 0 == first valid index
	while (this->incrementIndex(index))
	{
		if (otherGroup.hasIndex(index))
		{
			this->setIndex(index, false);
			++numberRemoved;
		}
	}
	return numberRemoved;
}

int DsLabelsGroup::setIndex(DsLabelIndex index, bool inGroup)
{
	if (index < 0)
	{
		display_message(ERROR_MESSAGE, "DsLabelsGroup::setIndex.  Invalid argument");
		return CMZN_ERROR_ARGUMENT;
	}
	bool wasInGroup;
	if (this->values.setBool(index, inGroup, wasInGroup))
	{
		if (inGroup != wasInGroup)
		{
			if (inGroup)
			{
				++labelsCount;
				if (index >= indexLimit)
					indexLimit = index + 1;
			}
			else
			{
				--labelsCount;
			}
			return CMZN_OK;
		}
		else if (inGroup)
			return CMZN_ERROR_ALREADY_EXISTS;
		return CMZN_ERROR_NOT_FOUND;
	}
	display_message(ERROR_MESSAGE, "DsLabelsGroup::setIndex.  Failed to set bool");
	return CMZN_ERROR_MEMORY;
}

/**
 * Get first label index in group or DS_LABEL_INDEX_INVALID if none.
 * Currently returns index with the lowest identifier in set.
 * Must pass in a newly initialised iterator, point to start.
 */
DsLabelIndex DsLabelsGroup::getFirstIndex(DsLabelIterator &iterator)
{
	DsLabelIndex index = iterator.getIndex();
	while ((index != DS_LABEL_INDEX_INVALID) && (!this->hasIndex(index)))
		index = iterator.nextIndex();
	return index;
}

DsLabelIterator *DsLabelsGroup::createLabelIterator()
{
	return this->labels->createLabelIterator(&(this->values));
}