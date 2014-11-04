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

int DsLabelsGroup::incrementLabelIterator(DsLabelIterator *iterator)
{
	if (!iterator || (iterator->getLabels() != this->labels))
		return 0;
	iterator->setIndex(getNextIndex(iterator->index));
	return (iterator->getIndex() >= 0);
}
