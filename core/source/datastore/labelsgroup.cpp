/**
 * FILE : datastore/labelsgroup.cpp
 * 
 * Implements a subset of a datastore labels set.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
}

DsLabelsGroup *DsLabelsGroup::create(DsLabels *labelsIn)
{
	return new DsLabelsGroup(labelsIn);
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

bool DsLabelsGroup::incrementLabelIterator(DsLabelIterator &iterator)
{
	if (iterator.getLabels() != this->labels)
		return 0;
	// this can be made more efficient
	DsLabelIndex index;
	while ((index = iterator.nextIndex()) != DS_LABEL_INDEX_INVALID)
	{
		if (this->values.getBool(index))
			return true;
	}
	return false;
}
